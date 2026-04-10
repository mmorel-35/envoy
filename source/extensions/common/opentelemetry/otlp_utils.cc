#include "source/extensions/common/opentelemetry/otlp_utils.h"

#include <string>

#include "source/common/common/macros.h"
#include "source/common/version/version.h"

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {

namespace {

enum OTelAttributeType {
  KTypeBool,
  KTypeInt,
  KTypeUInt,
  KTypeInt64,
  KTypeDouble,
  KTypeString,
  KTypeStringView,
  KTypeSpanBool,
  KTypeSpanInt,
  KTypeSpanUInt,
  KTypeSpanInt64,
  KTypeSpanDouble,
  KTypeSpanString,
  KTypeSpanStringView,
  KTypeUInt64,
  KTypeSpanUInt64,
  KTypeSpanByte
};

} // namespace

const std::string& OtlpUtils::getOtlpUserAgentHeader() {
  CONSTRUCT_ON_FIRST_USE(std::string, "OTel-OTLP-Exporter-Envoy/" + VersionInfo::version());
}

void OtlpUtils::populateAnyValue(AnyValue& value_proto,
                                 const Sdk::Common::AttributeValue& attribute_value) {
  switch (attribute_value.index()) {
  case OTelAttributeType::KTypeBool:
    value_proto.set_bool_value(absl::get<bool>(attribute_value) ? true : false);
    break;
  case OTelAttributeType::KTypeInt:
    value_proto.set_int_value(absl::get<int32_t>(attribute_value));
    break;
  case OTelAttributeType::KTypeInt64:
    value_proto.set_int_value(absl::get<int64_t>(attribute_value));
    break;
  case OTelAttributeType::KTypeUInt:
    value_proto.set_int_value(absl::get<uint32_t>(attribute_value));
    break;
  case OTelAttributeType::KTypeUInt64:
    value_proto.set_int_value(absl::get<uint64_t>(attribute_value));
    break;
  case OTelAttributeType::KTypeDouble:
    value_proto.set_double_value(absl::get<double>(attribute_value));
    break;
  case OTelAttributeType::KTypeString: {
    const auto sv = absl::get<std::string>(attribute_value);
    value_proto.set_string_value(sv.data(), sv.size());
    break;
  }
  case OTelAttributeType::KTypeStringView: {
    const auto sv = absl::get<absl::string_view>(attribute_value);
    value_proto.set_string_value(sv.data(), sv.size());
    break;
  }
  case OTelAttributeType::KTypeSpanString: {
    auto array_value = value_proto.mutable_array_value();
    for (const auto& val : absl::get<std::vector<std::string>>(attribute_value)) {
      array_value->add_values()->set_string_value(val.data(), val.size());
    }
    break;
  }
  case OTelAttributeType::KTypeSpanStringView: {
    auto array_value = value_proto.mutable_array_value();
    for (const auto& val : absl::get<std::vector<absl::string_view>>(attribute_value)) {
      array_value->add_values()->set_string_value(val.data(), val.size());
    }
    break;
  }
  default:
    IS_ENVOY_BUG("unexpected otel attribute type");
  }
}

KeyValue OtlpUtils::getStringKeyValue(const std::string& key, const std::string& value) {
  KeyValue key_value;
  key_value.set_key(key);
  key_value.mutable_value()->set_string_value(value);
  return key_value;
}

} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy
