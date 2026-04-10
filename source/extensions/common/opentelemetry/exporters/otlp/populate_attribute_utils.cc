#include "source/extensions/common/opentelemetry/exporters/otlp/populate_attribute_utils.h"

#include <cstdint>
#include <limits>
#include <string>

#include "source/common/common/assert.h"

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {
namespace Exporters {
namespace Otlp {

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

void PopulateAttributeUtils::populateAnyValue(AnyValue& value_proto,
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
  case OTelAttributeType::KTypeUInt64: {
    const uint64_t v = absl::get<uint64_t>(attribute_value);
    value_proto.set_int_value(v <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max())
                                  ? static_cast<int64_t>(v)
                                  : std::numeric_limits<int64_t>::max());
    break;
  }
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
  case OTelAttributeType::KTypeSpanBool: {
    auto* array_value = value_proto.mutable_array_value();
    for (const auto val : absl::get<std::vector<bool>>(attribute_value)) {
      array_value->add_values()->set_bool_value(val);
    }
    break;
  }
  case OTelAttributeType::KTypeSpanInt: {
    auto* array_value = value_proto.mutable_array_value();
    for (const auto val : absl::get<std::vector<int32_t>>(attribute_value)) {
      array_value->add_values()->set_int_value(val);
    }
    break;
  }
  case OTelAttributeType::KTypeSpanUInt: {
    auto* array_value = value_proto.mutable_array_value();
    for (const auto val : absl::get<std::vector<uint32_t>>(attribute_value)) {
      array_value->add_values()->set_int_value(val);
    }
    break;
  }
  case OTelAttributeType::KTypeSpanInt64: {
    auto* array_value = value_proto.mutable_array_value();
    for (const auto val : absl::get<std::vector<int64_t>>(attribute_value)) {
      array_value->add_values()->set_int_value(val);
    }
    break;
  }
  case OTelAttributeType::KTypeSpanDouble: {
    auto* array_value = value_proto.mutable_array_value();
    for (const auto val : absl::get<std::vector<double>>(attribute_value)) {
      array_value->add_values()->set_double_value(val);
    }
    break;
  }
  case OTelAttributeType::KTypeSpanString: {
    auto* array_value = value_proto.mutable_array_value();
    for (const auto& val : absl::get<std::vector<std::string>>(attribute_value)) {
      array_value->add_values()->set_string_value(val.data(), val.size());
    }
    break;
  }
  case OTelAttributeType::KTypeSpanStringView: {
    auto* array_value = value_proto.mutable_array_value();
    for (const auto& val : absl::get<std::vector<absl::string_view>>(attribute_value)) {
      array_value->add_values()->set_string_value(val.data(), val.size());
    }
    break;
  }
  case OTelAttributeType::KTypeSpanUInt64: {
    auto* array_value = value_proto.mutable_array_value();
    for (const uint64_t val : absl::get<std::vector<uint64_t>>(attribute_value)) {
      array_value->add_values()->set_int_value(
          val <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max())
              ? static_cast<int64_t>(val)
              : std::numeric_limits<int64_t>::max());
    }
    break;
  }
  case OTelAttributeType::KTypeSpanByte: {
    const auto& bytes = absl::get<std::vector<uint8_t>>(attribute_value);
    value_proto.set_bytes_value(bytes.data(), bytes.size());
    break;
  }
  default:
    IS_ENVOY_BUG("unexpected otel attribute type");
  }
}

KeyValue PopulateAttributeUtils::makeKeyValue(const std::string& key, const std::string& value) {
  KeyValue key_value;
  key_value.set_key(key);
  key_value.mutable_value()->set_string_value(value);
  return key_value;
}

} // namespace Otlp
} // namespace Exporters
} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy
