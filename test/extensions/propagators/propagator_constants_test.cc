#include "source/extensions/propagators/propagator_constants.h"

#include "gtest/gtest.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace {

class PropagatorConstantsTest : public testing::Test {};

TEST_F(PropagatorConstantsTest, Constants) {
  const auto& constants = PropagatorConstants::get();

  // Test B3 constants
  EXPECT_EQ(constants.X_B3_TRACE_ID.key().get(), "x-b3-traceid");
  EXPECT_EQ(constants.X_B3_SPAN_ID.key().get(), "x-b3-spanid");
  EXPECT_EQ(constants.X_B3_PARENT_SPAN_ID.key().get(), "x-b3-parentspanid");
  EXPECT_EQ(constants.X_B3_SAMPLED.key().get(), "x-b3-sampled");
  EXPECT_EQ(constants.X_B3_FLAGS.key().get(), "x-b3-flags");
  EXPECT_EQ(constants.B3.key().get(), "b3");

  // Test W3C constants
  EXPECT_EQ(constants.TRACE_PARENT.key().get(), "traceparent");
  EXPECT_EQ(constants.TRACE_STATE.key().get(), "tracestate");

  // Test X-Ray constants
  EXPECT_EQ(constants.X_AMZN_TRACE_ID.key().get(), "x-amzn-trace-id");
}

} // namespace
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy