#pragma once

#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/string_view.h"
#include "absl/types/variant.h"
#include "opentelemetry/proto/common/v1/common.pb.h"
#include "opentelemetry/proto/trace/v1/trace.pb.h"

namespace Envoy {
namespace Extensions {
namespace OpenTelemetry {

/**
 * @brief The type of the span.
 * see
 * https://github.com/open-telemetry/opentelemetry-specification/blob/main/specification/trace/api.md#spankind
 */
using OTelSpanKind = ::opentelemetry::proto::trace::v1::Span::SpanKind;

/**
 * @brief Based on Open-telemetry OwnedAttributeValue
 * see
 * https://github.com/open-telemetry/opentelemetry-cpp/blob/main/sdk/include/opentelemetry/sdk/common/attribute_utils.h
 */
using OTelAttribute =
    absl::variant<bool, int32_t, uint32_t, int64_t, double, std::string, absl::string_view,
                  std::vector<bool>, std::vector<int32_t>, std::vector<uint32_t>,
                  std::vector<int64_t>, std::vector<double>, std::vector<std::string>,
                  std::vector<absl::string_view>, uint64_t, std::vector<uint64_t>,
                  std::vector<uint8_t>>;

/**
 * @brief Container holding Open-telemetry Attributes
 */
using OtelAttributes = absl::flat_hash_map<std::string, OTelAttribute>;

using KeyValue = ::opentelemetry::proto::common::v1::KeyValue;
using AnyValue = ::opentelemetry::proto::common::v1::AnyValue;

} // namespace OpenTelemetry
} // namespace Extensions
} // namespace Envoy
