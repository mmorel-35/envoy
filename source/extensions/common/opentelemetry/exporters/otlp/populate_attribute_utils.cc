#include "source/extensions/common/opentelemetry/exporters/otlp/populate_attribute_utils.h"

#include <string>
#include <type_traits>

#include "source/common/common/assert.h"

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {
namespace Exporters {
namespace Otlp {

void PopulateAttributeUtils::populateAnyValue(AnyValue& value_proto,
                                              const Sdk::Common::AttributeValue& attribute_value) {
  absl::visit(
      [&value_proto](const auto& val) {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, bool>) {
          value_proto.set_bool_value(val);
        } else if constexpr (std::is_same_v<T, int32_t>) {
          value_proto.set_int_value(val);
        } else if constexpr (std::is_same_v<T, int64_t>) {
          value_proto.set_int_value(val);
        } else if constexpr (std::is_same_v<T, uint32_t>) {
          value_proto.set_int_value(val);
        } else if constexpr (std::is_same_v<T, uint64_t>) {
          value_proto.set_int_value(static_cast<int64_t>(val));
        } else if constexpr (std::is_same_v<T, double>) {
          value_proto.set_double_value(val);
        } else if constexpr (std::is_same_v<T, std::string>) {
          value_proto.set_string_value(val.data(), val.size());
        } else if constexpr (std::is_same_v<T, absl::string_view>) {
          value_proto.set_string_value(val.data(), val.size());
        } else if constexpr (std::is_same_v<T, std::vector<bool>>) {
          auto* array_value = value_proto.mutable_array_value();
          for (const auto elem : val) {
            array_value->add_values()->set_bool_value(elem);
          }
        } else if constexpr (std::is_same_v<T, std::vector<int32_t>>) {
          auto* array_value = value_proto.mutable_array_value();
          for (const auto elem : val) {
            array_value->add_values()->set_int_value(elem);
          }
        } else if constexpr (std::is_same_v<T, std::vector<uint32_t>>) {
          auto* array_value = value_proto.mutable_array_value();
          for (const auto elem : val) {
            array_value->add_values()->set_int_value(elem);
          }
        } else if constexpr (std::is_same_v<T, std::vector<int64_t>>) {
          auto* array_value = value_proto.mutable_array_value();
          for (const auto elem : val) {
            array_value->add_values()->set_int_value(elem);
          }
        } else if constexpr (std::is_same_v<T, std::vector<double>>) {
          auto* array_value = value_proto.mutable_array_value();
          for (const auto elem : val) {
            array_value->add_values()->set_double_value(elem);
          }
        } else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
          auto* array_value = value_proto.mutable_array_value();
          for (const auto& elem : val) {
            array_value->add_values()->set_string_value(elem.data(), elem.size());
          }
        } else if constexpr (std::is_same_v<T, std::vector<absl::string_view>>) {
          auto* array_value = value_proto.mutable_array_value();
          for (const auto& elem : val) {
            array_value->add_values()->set_string_value(elem.data(), elem.size());
          }
        } else if constexpr (std::is_same_v<T, std::vector<uint64_t>>) {
          auto* array_value = value_proto.mutable_array_value();
          for (const uint64_t elem : val) {
            array_value->add_values()->set_int_value(static_cast<int64_t>(elem));
          }
        } else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
          value_proto.set_bytes_value(val.data(), val.size());
        } else {
          IS_ENVOY_BUG("unexpected otel attribute type");
        }
      },
      attribute_value);
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
