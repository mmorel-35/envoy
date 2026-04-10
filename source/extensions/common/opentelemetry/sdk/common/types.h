#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/string_view.h"
#include "absl/types/variant.h"

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {
namespace Sdk {
namespace Common {

/**
 * @brief Envoy's representation of an OpenTelemetry attribute value.
 *
 * Mirrors opentelemetry::sdk::common::OwnedAttributeValue from opentelemetry-cpp, adapted to use
 * Abseil types available in Envoy's build environment rather than the upstream's type-list.
 * Note: like the upstream type, this variant includes both owning (std::string, std::vector) and
 * non-owning (absl::string_view) alternatives. Callers that need to retain values across scope
 * boundaries must store std::string, not absl::string_view.
 * @see
 * https://github.com/open-telemetry/opentelemetry-cpp/blob/main/sdk/include/opentelemetry/sdk/common/attribute_utils.h
 */
using AttributeValue =
    absl::variant<bool, int32_t, uint32_t, int64_t, double, std::string, absl::string_view,
                  std::vector<bool>, std::vector<int32_t>, std::vector<uint32_t>,
                  std::vector<int64_t>, std::vector<double>, std::vector<std::string>,
                  std::vector<absl::string_view>, uint64_t, std::vector<uint64_t>,
                  std::vector<uint8_t>>;

/**
 * @brief A map of OpenTelemetry attribute key-value pairs.
 *
 * Mirrors opentelemetry::sdk::common::OrderedAttributeMap / AttributeMap, using Abseil's
 * flat_hash_map for efficient lookups in Envoy's hot paths.
 */
using OwnedAttributeMap = absl::flat_hash_map<std::string, AttributeValue>;

} // namespace Common
} // namespace Sdk
} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy
