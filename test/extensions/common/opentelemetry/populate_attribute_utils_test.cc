#include <type_traits>

#include "source/extensions/common/opentelemetry/exporters/otlp/environment.h"
#include "source/extensions/common/opentelemetry/populate_attribute_utils.h"
#include "source/extensions/common/opentelemetry/types.h"

#include "test/test_common/utility.h"

#include "absl/strings/match.h"
#include "opentelemetry/proto/common/v1/common.pb.h"

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {
namespace {

TEST(ExporterEnvironmentTest, GetUserAgent) {
  const auto& ua = Exporters::Otlp::GetUserAgent();
  EXPECT_TRUE(absl::StartsWith(ua, "OTel-OTLP-Exporter-Envoy/"));
  // Should return the same instance each time (CONSTRUCT_ON_FIRST_USE).
  EXPECT_EQ(&ua, &Exporters::Otlp::GetUserAgent());
}

TEST(PopulateAttributeUtilsTest, MakeKeyValueSetsKeyAndStringValue) {
  auto kv = PopulateAttributeUtils::makeKeyValue("my_key", "my_value");
  EXPECT_EQ("my_key", kv.key());
  EXPECT_EQ("my_value", kv.value().string_value());
}

TEST(PopulateAttributeUtilsTest, MakeKeyValueEmptyStrings) {
  auto kv = PopulateAttributeUtils::makeKeyValue("", "");
  EXPECT_EQ("", kv.key());
  EXPECT_EQ("", kv.value().string_value());
}

TEST(PopulateAttributeUtilsTest, PopulateAnyValueBool) {
  AnyValue proto;
  OTelAttribute attr = true;
  PopulateAttributeUtils::populateAnyValue(proto, attr);
  EXPECT_TRUE(proto.bool_value());
}

TEST(PopulateAttributeUtilsTest, PopulateAnyValueInt32) {
  AnyValue proto;
  OTelAttribute attr = static_cast<int32_t>(42);
  PopulateAttributeUtils::populateAnyValue(proto, attr);
  EXPECT_EQ(42, proto.int_value());
}

TEST(PopulateAttributeUtilsTest, PopulateAnyValueInt64) {
  AnyValue proto;
  OTelAttribute attr = static_cast<int64_t>(123456789LL);
  PopulateAttributeUtils::populateAnyValue(proto, attr);
  EXPECT_EQ(123456789LL, proto.int_value());
}

TEST(PopulateAttributeUtilsTest, PopulateAnyValueUInt32) {
  AnyValue proto;
  OTelAttribute attr = static_cast<uint32_t>(99);
  PopulateAttributeUtils::populateAnyValue(proto, attr);
  EXPECT_EQ(99, proto.int_value());
}

TEST(PopulateAttributeUtilsTest, PopulateAnyValueUInt64) {
  AnyValue proto;
  OTelAttribute attr = static_cast<uint64_t>(999999999999ULL);
  PopulateAttributeUtils::populateAnyValue(proto, attr);
  EXPECT_EQ(static_cast<int64_t>(999999999999ULL), proto.int_value());
}

TEST(PopulateAttributeUtilsTest, PopulateAnyValueDouble) {
  AnyValue proto;
  OTelAttribute attr = 3.14;
  PopulateAttributeUtils::populateAnyValue(proto, attr);
  EXPECT_DOUBLE_EQ(3.14, proto.double_value());
}

TEST(PopulateAttributeUtilsTest, PopulateAnyValueString) {
  AnyValue proto;
  OTelAttribute attr = std::string("hello");
  PopulateAttributeUtils::populateAnyValue(proto, attr);
  EXPECT_EQ("hello", proto.string_value());
}

TEST(PopulateAttributeUtilsTest, PopulateAnyValueStringView) {
  AnyValue proto;
  OTelAttribute attr = absl::string_view("world");
  PopulateAttributeUtils::populateAnyValue(proto, attr);
  EXPECT_EQ("world", proto.string_value());
}

TEST(PopulateAttributeUtilsTest, PopulateAnyValueVectorString) {
  AnyValue proto;
  OTelAttribute attr = std::vector<std::string>{"a", "b", "c"};
  PopulateAttributeUtils::populateAnyValue(proto, attr);
  ASSERT_EQ(3, proto.array_value().values_size());
  EXPECT_EQ("a", proto.array_value().values(0).string_value());
  EXPECT_EQ("b", proto.array_value().values(1).string_value());
  EXPECT_EQ("c", proto.array_value().values(2).string_value());
}

TEST(PopulateAttributeUtilsTest, PopulateAnyValueVectorStringView) {
  AnyValue proto;
  OTelAttribute attr = std::vector<absl::string_view>{"x", "y"};
  PopulateAttributeUtils::populateAnyValue(proto, attr);
  ASSERT_EQ(2, proto.array_value().values_size());
  EXPECT_EQ("x", proto.array_value().values(0).string_value());
  EXPECT_EQ("y", proto.array_value().values(1).string_value());
}

TEST(PopulateAttributeUtilsTest, OtelAttributesUsesAbslFlatHashMap) {
  // Compile-time check: OtelAttributes must be exactly absl::flat_hash_map<std::string,
  // OTelAttribute>.
  static_assert(std::is_same_v<OtelAttributes, absl::flat_hash_map<std::string, OTelAttribute>>,
                "OtelAttributes must be absl::flat_hash_map<std::string, OTelAttribute>");

  OtelAttributes attrs;
  attrs["key1"] = std::string("val1");
  attrs["key2"] = static_cast<int32_t>(2);
  EXPECT_EQ(2u, attrs.size());
  EXPECT_TRUE(attrs.contains("key1"));
  EXPECT_TRUE(attrs.contains("key2"));
}

} // namespace
} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy
