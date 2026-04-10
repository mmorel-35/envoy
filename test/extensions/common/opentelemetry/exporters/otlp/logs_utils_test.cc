#include "source/common/router/string_accessor_impl.h"
#include "source/common/stream_info/filter_state_impl.h"
#include "source/extensions/common/opentelemetry/exporters/otlp/logs_utils.h"

#include "test/mocks/local_info/mocks.h"
#include "test/mocks/stream_info/mocks.h"
#include "test/test_common/utility.h"

#include "absl/strings/escaping.h"
#include "absl/strings/match.h"
#include "opentelemetry/proto/collector/logs/v1/logs_service.pb.h"
#include "opentelemetry/proto/resource/v1/resource.pb.h"

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {
namespace Exporters {
namespace Otlp {
namespace {

using testing::NiceMock;
using testing::Return;
using testing::ReturnRef;

const std::string kTestZone = "test_zone";
const std::string kTestCluster = "test_cluster";
const std::string kTestNode = "test_node";

TEST(LogsUtilsTest, GetStringKeyValue) {
  auto kv = getStringKeyValue("test_key", "test_value");
  EXPECT_EQ("test_key", kv.key());
  EXPECT_EQ("test_value", kv.value().string_value());
}

TEST(LogsUtilsTest, PackUnpackBody) {
  ::opentelemetry::proto::common::v1::AnyValue body;
  body.set_string_value("test body content");

  auto packed = packBody(body);
  ASSERT_EQ(1, packed.values().size());
  EXPECT_EQ(BodyKey, packed.values(0).key());

  auto unpacked = unpackBody(packed);
  EXPECT_EQ("test body content", unpacked.string_value());
}

TEST(LogsUtilsTest, GetOtlpUserAgentHeader) {
  const auto& header = getOtlpUserAgentHeader();
  EXPECT_TRUE(absl::StartsWith(header, "OTel-OTLP-Exporter-Envoy/"));
  // Should return the same instance each time.
  EXPECT_EQ(&header, &getOtlpUserAgentHeader());
}

TEST(LogsUtilsTest, PopulateTraceContextFullTraceId) {
  opentelemetry::proto::logs::v1::LogRecord log_entry;
  // 32-char (128-bit) trace ID.
  const std::string trace_id_hex = "0123456789abcdef0123456789abcdef";
  const std::string span_id_hex = "0123456789abcdef";

  populateTraceContext(log_entry, trace_id_hex, span_id_hex);

  EXPECT_EQ(16, log_entry.trace_id().size());
  EXPECT_EQ(8, log_entry.span_id().size());
  EXPECT_EQ(absl::HexStringToBytes(trace_id_hex), log_entry.trace_id());
  EXPECT_EQ(absl::HexStringToBytes(span_id_hex), log_entry.span_id());
}

TEST(LogsUtilsTest, PopulateTraceContextShortTraceId) {
  opentelemetry::proto::logs::v1::LogRecord log_entry;
  // 16-char (64-bit, Zipkin-style) trace ID.
  const std::string short_trace_id_hex = "0123456789abcdef";
  const std::string span_id_hex = "fedcba9876543210";

  populateTraceContext(log_entry, short_trace_id_hex, span_id_hex);

  EXPECT_EQ(16, log_entry.trace_id().size());
  EXPECT_EQ(8, log_entry.span_id().size());
  // Should be padded with zeros on the left.
  const std::string expected_trace_id = "0000000000000000" + short_trace_id_hex;
  EXPECT_EQ(absl::HexStringToBytes(expected_trace_id), log_entry.trace_id());
}

TEST(LogsUtilsTest, PopulateTraceContextEmptyIds) {
  opentelemetry::proto::logs::v1::LogRecord log_entry;

  populateTraceContext(log_entry, "", "");

  EXPECT_TRUE(log_entry.trace_id().empty());
  EXPECT_TRUE(log_entry.span_id().empty());
}

TEST(LogsUtilsTest, PopulateTraceContextInvalidTraceIdLength) {
  opentelemetry::proto::logs::v1::LogRecord log_entry;
  // Invalid length (not 16 or 32 chars).
  const std::string invalid_trace_id = "0123456789";
  const std::string span_id_hex = "0123456789abcdef";

  populateTraceContext(log_entry, invalid_trace_id, span_id_hex);

  EXPECT_TRUE(log_entry.trace_id().empty());
  EXPECT_EQ(8, log_entry.span_id().size());
}

TEST(LogsUtilsTest, GetLogNamePrefersTopLevel) {
  envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig config;
  config.set_log_name("top_level_log");
  config.mutable_common_config()->set_log_name("common_config_log");

  EXPECT_EQ("top_level_log", getLogName(config));
}

TEST(LogsUtilsTest, GetLogNameFallsBackToCommonConfig) {
  envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig config;
  config.mutable_common_config()->set_log_name("common_config_log");

  EXPECT_EQ("common_config_log", getLogName(config));
}

TEST(LogsUtilsTest, GetLogNameReturnsEmptyWhenNotSet) {
  envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig config;

  EXPECT_TRUE(getLogName(config).empty());
}

TEST(LogsUtilsTest, GetGrpcServicePrefersTopLevel) {
  envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig config;
  config.mutable_grpc_service()->mutable_envoy_grpc()->set_cluster_name("top_level_cluster");
  config.mutable_common_config()->mutable_grpc_service()->mutable_envoy_grpc()->set_cluster_name(
      "common_config_cluster");

  EXPECT_EQ("top_level_cluster", getGrpcService(config).envoy_grpc().cluster_name());
}

TEST(LogsUtilsTest, GetGrpcServiceFallsBackToCommonConfig) {
  envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig config;
  config.mutable_common_config()->mutable_grpc_service()->mutable_envoy_grpc()->set_cluster_name(
      "common_config_cluster");

  EXPECT_EQ("common_config_cluster", getGrpcService(config).envoy_grpc().cluster_name());
}

TEST(LogsUtilsTest, GetBufferFlushIntervalFromConfig) {
  envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig config;
  config.mutable_buffer_flush_interval()->set_seconds(5);

  EXPECT_EQ(std::chrono::milliseconds(5000), getBufferFlushInterval(config));
}

TEST(LogsUtilsTest, GetBufferFlushIntervalReturnsDefaultWhenNotSet) {
  envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig config;

  EXPECT_EQ(DefaultBufferFlushInterval, getBufferFlushInterval(config));
}

TEST(LogsUtilsTest, GetBufferSizeBytesFromConfig) {
  envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig config;
  config.mutable_buffer_size_bytes()->set_value(32768);

  EXPECT_EQ(32768U, getBufferSizeBytes(config));
}

TEST(LogsUtilsTest, GetBufferSizeBytesReturnsDefaultWhenNotSet) {
  envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig config;

  EXPECT_EQ(DefaultMaxBufferSizeBytes, getBufferSizeBytes(config));
}

TEST(LogsUtilsTest, GetFilterStateObjectsToLogFromConfig) {
  envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig config;
  config.add_filter_state_objects_to_log("obj1");
  config.add_filter_state_objects_to_log("obj2");

  auto result = getFilterStateObjectsToLog(config);
  ASSERT_EQ(2U, result.size());
  EXPECT_EQ("obj1", result[0]);
  EXPECT_EQ("obj2", result[1]);
}

TEST(LogsUtilsTest, GetFilterStateObjectsToLogReturnsEmptyWhenNotSet) {
  envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig config;

  EXPECT_TRUE(getFilterStateObjectsToLog(config).empty());
}

TEST(LogsUtilsTest, GetCustomTagsFromConfig) {
  envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig config;
  auto* tag1 = config.add_custom_tags();
  tag1->set_tag("tag1");
  tag1->mutable_literal()->set_value("value1");

  auto result = getCustomTags(config);
  ASSERT_EQ(1U, result.size());
  EXPECT_EQ("tag1", result[0]->tag());
}

TEST(LogsUtilsTest, GetCustomTagsReturnsEmptyWhenNotSet) {
  envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig config;

  EXPECT_TRUE(getCustomTags(config).empty());
}

TEST(LogsUtilsTest, AddFilterStateToAttributesFromDownstream) {
  NiceMock<StreamInfo::MockStreamInfo> stream_info;
  opentelemetry::proto::logs::v1::LogRecord log_entry;

  stream_info.filter_state_->setData(
      "downstream_key", std::make_unique<Router::StringAccessorImpl>("downstream_value"),
      StreamInfo::FilterState::StateType::ReadOnly,
      StreamInfo::FilterState::LifeSpan::FilterChain);

  std::vector<std::string> filter_state_objects = {"downstream_key"};
  addFilterStateToAttributes(stream_info, filter_state_objects, log_entry);

  ASSERT_EQ(1, log_entry.attributes_size());
  EXPECT_EQ("downstream_key", log_entry.attributes(0).key());
  EXPECT_FALSE(log_entry.attributes(0).value().string_value().empty());
}

TEST(LogsUtilsTest, AddFilterStateToAttributesFromUpstream) {
  NiceMock<StreamInfo::MockStreamInfo> stream_info;
  opentelemetry::proto::logs::v1::LogRecord log_entry;

  auto upstream_filter_state = std::make_shared<StreamInfo::FilterStateImpl>(
      StreamInfo::FilterState::LifeSpan::FilterChain);
  upstream_filter_state->setData(
      "upstream_key", std::make_unique<Router::StringAccessorImpl>("upstream_value"),
      StreamInfo::FilterState::StateType::ReadOnly,
      StreamInfo::FilterState::LifeSpan::FilterChain);
  stream_info.upstreamInfo()->setUpstreamFilterState(upstream_filter_state);

  std::vector<std::string> filter_state_objects = {"upstream_key"};
  addFilterStateToAttributes(stream_info, filter_state_objects, log_entry);

  ASSERT_EQ(1, log_entry.attributes_size());
  EXPECT_EQ("upstream_key", log_entry.attributes(0).key());
  EXPECT_FALSE(log_entry.attributes(0).value().string_value().empty());
}

TEST(LogsUtilsTest, AddFilterStateToAttributesMissingKey) {
  NiceMock<StreamInfo::MockStreamInfo> stream_info;
  opentelemetry::proto::logs::v1::LogRecord log_entry;

  std::vector<std::string> filter_state_objects = {"nonexistent_key"};
  addFilterStateToAttributes(stream_info, filter_state_objects, log_entry);

  EXPECT_EQ(0, log_entry.attributes_size());
}

TEST(LogsUtilsTest, AddFilterStateToAttributesEmptyList) {
  NiceMock<StreamInfo::MockStreamInfo> stream_info;
  opentelemetry::proto::logs::v1::LogRecord log_entry;

  addFilterStateToAttributes(stream_info, {}, log_entry);

  EXPECT_EQ(0, log_entry.attributes_size());
}

TEST(LogsUtilsTest, AddCustomTagsToAttributesWithLiteralTags) {
  NiceMock<StreamInfo::MockStreamInfo> stream_info;
  opentelemetry::proto::logs::v1::LogRecord log_entry;

  envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig config;
  auto* tag = config.add_custom_tags();
  tag->set_tag("literal_tag");
  tag->mutable_literal()->set_value("literal_value");
  auto custom_tags = getCustomTags(config);

  Http::TestRequestHeaderMapImpl request_headers;
  Formatter::Context context(&request_headers);

  addCustomTagsToAttributes(custom_tags, context, stream_info, log_entry);

  opentelemetry::proto::logs::v1::LogRecord expected;
  auto* attr = expected.add_attributes();
  attr->set_key("literal_tag");
  attr->mutable_value()->set_string_value("literal_value");

  EXPECT_TRUE(TestUtility::protoEqual(log_entry, expected));
}

TEST(LogsUtilsTest, AddCustomTagsToAttributesEmptyTags) {
  NiceMock<StreamInfo::MockStreamInfo> stream_info;
  opentelemetry::proto::logs::v1::LogRecord log_entry;

  Http::TestRequestHeaderMapImpl request_headers;
  Formatter::Context context(&request_headers);

  addCustomTagsToAttributes({}, context, stream_info, log_entry);

  EXPECT_TRUE(TestUtility::protoEqual(log_entry, {}));
}

TEST(LogsUtilsTest, AddCustomTagsToAttributesWithoutRequestHeaders) {
  NiceMock<StreamInfo::MockStreamInfo> stream_info;
  opentelemetry::proto::logs::v1::LogRecord log_entry;

  envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig config;
  auto* tag = config.add_custom_tags();
  tag->set_tag("env_tag");
  tag->mutable_literal()->set_value("env_value");
  auto custom_tags = getCustomTags(config);

  // No request headers (e.g. TCP context).
  Formatter::Context context;

  addCustomTagsToAttributes(custom_tags, context, stream_info, log_entry);

  opentelemetry::proto::logs::v1::LogRecord expected;
  auto* attr = expected.add_attributes();
  attr->set_key("env_tag");
  attr->mutable_value()->set_string_value("env_value");

  EXPECT_TRUE(TestUtility::protoEqual(log_entry, expected));
}

TEST(LogsUtilsTest, InitOtlpMessageRootWithBuiltinLabels) {
  opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest message;
  envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig config;
  config.set_log_name("test_log");

  NiceMock<LocalInfo::MockLocalInfo> local_info;
  ON_CALL(local_info, zoneName()).WillByDefault(ReturnRef(kTestZone));
  ON_CALL(local_info, clusterName()).WillByDefault(ReturnRef(kTestCluster));
  ON_CALL(local_info, nodeName()).WillByDefault(ReturnRef(kTestNode));

  auto* root = initOtlpMessageRoot(message, config, local_info);

  ASSERT_NE(nullptr, root);
  ASSERT_EQ(1, message.resource_logs_size());

  opentelemetry::proto::resource::v1::Resource expected_resource;
  auto* attr = expected_resource.add_attributes();
  attr->set_key("log_name");
  attr->mutable_value()->set_string_value("test_log");
  attr = expected_resource.add_attributes();
  attr->set_key("zone_name");
  attr->mutable_value()->set_string_value(kTestZone);
  attr = expected_resource.add_attributes();
  attr->set_key("cluster_name");
  attr->mutable_value()->set_string_value(kTestCluster);
  attr = expected_resource.add_attributes();
  attr->set_key("node_name");
  attr->mutable_value()->set_string_value(kTestNode);

  EXPECT_TRUE(TestUtility::protoEqual(message.resource_logs(0).resource(), expected_resource));
}

TEST(LogsUtilsTest, InitOtlpMessageRootDisableBuiltinLabels) {
  opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest message;
  envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig config;
  config.set_disable_builtin_labels(true);

  NiceMock<LocalInfo::MockLocalInfo> local_info;

  auto* root = initOtlpMessageRoot(message, config, local_info);

  ASSERT_NE(nullptr, root);
  ASSERT_EQ(1, message.resource_logs_size());
  EXPECT_TRUE(TestUtility::protoEqual(message.resource_logs(0).resource(), {}));
}

TEST(LogsUtilsTest, InitOtlpMessageRootWithResourceAttributes) {
  opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest message;
  envoy::extensions::access_loggers::open_telemetry::v3::OpenTelemetryAccessLogConfig config;
  config.set_disable_builtin_labels(true);
  auto* kv = config.mutable_resource_attributes()->add_values();
  kv->set_key("custom_key");
  kv->mutable_value()->set_string_value("custom_value");

  NiceMock<LocalInfo::MockLocalInfo> local_info;

  auto* root = initOtlpMessageRoot(message, config, local_info);

  ASSERT_NE(nullptr, root);

  opentelemetry::proto::resource::v1::Resource expected_resource;
  auto* attr = expected_resource.add_attributes();
  attr->set_key("custom_key");
  attr->mutable_value()->set_string_value("custom_value");

  EXPECT_TRUE(TestUtility::protoEqual(message.resource_logs(0).resource(), expected_resource));
}

} // namespace
} // namespace Otlp
} // namespace Exporters
} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy
