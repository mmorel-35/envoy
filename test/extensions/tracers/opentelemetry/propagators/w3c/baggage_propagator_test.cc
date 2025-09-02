#include "source/extensions/tracers/opentelemetry/propagators/w3c/baggage_propagator.h"

#include "test/test_common/utility.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace OpenTelemetry {

using testing::HasSubstr;

class BaggagePropagatorTest : public testing::Test {
public:
  BaggagePropagatorTest() : propagator_(std::make_unique<BaggagePropagator>()) {}

protected:
  std::unique_ptr<BaggagePropagator> propagator_;
};

TEST_F(BaggagePropagatorTest, ExtractAlwaysFails) {
  // Baggage propagator doesn't extract trace context, only baggage data
  Tracing::TestTraceContextImpl trace_context{{"baggage", "key1=value1,key2=value2"}};

  auto result = propagator_->extract(trace_context);

  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.status().message(),
              HasSubstr("Baggage propagator cannot extract span context"));
}

TEST_F(BaggagePropagatorTest, ExtractFailsWithoutBaggageHeader) {
  Tracing::TestTraceContextImpl trace_context{{"other-header", "value"}};

  auto result = propagator_->extract(trace_context);

  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.status().message(),
              HasSubstr("Baggage propagator cannot extract span context"));
}

TEST_F(BaggagePropagatorTest, InjectDoesNothing) {
  // Baggage propagator doesn't inject trace context
  SpanContext span_context("00000000000000000000000000000001", "0000000000000002", true, "");
  Tracing::TestTraceContextImpl trace_context{};

  propagator_->inject(span_context, trace_context);

  // Should not inject any headers
  EXPECT_TRUE(trace_context.get("baggage").empty());
  EXPECT_TRUE(trace_context.get("traceparent").empty());
}

TEST_F(BaggagePropagatorTest, FieldsReturnsBaggageHeader) {
  auto fields = propagator_->fields();

  EXPECT_EQ(fields.size(), 1);
  EXPECT_THAT(fields, testing::Contains("baggage"));
}

TEST_F(BaggagePropagatorTest, NameReturnsBaggage) { EXPECT_EQ(propagator_->name(), "baggage"); }

} // namespace OpenTelemetry
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy
