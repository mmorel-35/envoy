#include "source/extensions/propagators/w3c/baggage/baggage_propagator.h"

#include "test/test_common/utility.h"

#include "gtest/gtest.h"

namespace Envoy {
namespace Extensions {
namespace Propagators {
namespace W3c {
namespace Baggage {
namespace {

class BaggagePropagatorTest : public testing::Test {
protected:
  BaggagePropagator propagator_;
  
  // Valid test values per W3C specification
  const std::string simple_baggage_ = "key1=value1";
  const std::string multi_baggage_ = "key1=value1,key2=value2";
  const std::string baggage_with_properties_ = "key1=value1;property1,key2=value2;property2;property3";
  const std::string complex_baggage_ = "userId=alice,serverNode=DF%2028,isProduction=false";
};

TEST_F(BaggagePropagatorTest, ExtractBaggageSuccess) {
  Tracing::TestTraceContextImpl trace_context{{"baggage", simple_baggage_}};
  
  auto result = propagator_.extractBaggage(trace_context);
  
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), simple_baggage_);
}

TEST_F(BaggagePropagatorTest, ExtractBaggageNotPresent) {
  Tracing::TestTraceContextImpl trace_context{};
  
  auto result = propagator_.extractBaggage(trace_context);
  
  EXPECT_FALSE(result.has_value());
}

TEST_F(BaggagePropagatorTest, ExtractBaggageMultipleValues) {
  Tracing::TestTraceContextImpl trace_context{{"baggage", "key1=value1"},
                                               {"baggage", "key2=value2"}};
  
  auto result = propagator_.extractBaggage(trace_context);
  
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), "key1=value1,key2=value2");
}

TEST_F(BaggagePropagatorTest, ParseBaggageEmpty) {
  auto result = propagator_.parseBaggage("");
  
  ASSERT_TRUE(result.ok());
  EXPECT_TRUE(result->empty());
}

TEST_F(BaggagePropagatorTest, ParseBaggageSimple) {
  auto result = propagator_.parseBaggage(simple_baggage_);
  
  ASSERT_TRUE(result.ok());
  ASSERT_EQ(result->size(), 1);
  EXPECT_EQ(result->at("key1").key, "key1");
  EXPECT_EQ(result->at("key1").value, "value1");
  EXPECT_TRUE(result->at("key1").properties.empty());
}

TEST_F(BaggagePropagatorTest, ParseBaggageMultiple) {
  auto result = propagator_.parseBaggage(multi_baggage_);
  
  ASSERT_TRUE(result.ok());
  ASSERT_EQ(result->size(), 2);
  
  EXPECT_EQ(result->at("key1").key, "key1");
  EXPECT_EQ(result->at("key1").value, "value1");
  
  EXPECT_EQ(result->at("key2").key, "key2");
  EXPECT_EQ(result->at("key2").value, "value2");
}

TEST_F(BaggagePropagatorTest, ParseBaggageWithProperties) {
  auto result = propagator_.parseBaggage(baggage_with_properties_);
  
  ASSERT_TRUE(result.ok());
  ASSERT_EQ(result->size(), 2);
  
  EXPECT_EQ(result->at("key1").key, "key1");
  EXPECT_EQ(result->at("key1").value, "value1");
  ASSERT_EQ(result->at("key1").properties.size(), 1);
  EXPECT_EQ(result->at("key1").properties[0], "property1");
  
  EXPECT_EQ(result->at("key2").key, "key2");
  EXPECT_EQ(result->at("key2").value, "value2");
  ASSERT_EQ(result->at("key2").properties.size(), 2);
  EXPECT_EQ(result->at("key2").properties[0], "property2");
  EXPECT_EQ(result->at("key2").properties[1], "property3");
}

TEST_F(BaggagePropagatorTest, ParseBaggageComplex) {
  auto result = propagator_.parseBaggage(complex_baggage_);
  
  ASSERT_TRUE(result.ok());
  ASSERT_EQ(result->size(), 3);
  
  EXPECT_EQ(result->at("userId").value, "alice");
  EXPECT_EQ(result->at("serverNode").value, "DF%2028");
  EXPECT_EQ(result->at("isProduction").value, "false");
}

TEST_F(BaggagePropagatorTest, ParseBaggageInvalidKeyValue) {
  // Missing value
  auto result1 = propagator_.parseBaggage("key1");
  EXPECT_FALSE(result1.ok());
  EXPECT_THAT(result1.status().message(), testing::HasSubstr("Invalid baggage key=value format"));
  
  // Missing key
  auto result2 = propagator_.parseBaggage("=value1");
  EXPECT_FALSE(result2.ok());
  
  // Multiple equals
  auto result3 = propagator_.parseBaggage("key1=value1=extra");
  EXPECT_FALSE(result3.ok());
}

TEST_F(BaggagePropagatorTest, ParseBaggageTooLarge) {
  // Create baggage that exceeds max size
  std::string large_baggage(Constants::kMaxBaggageSize + 1, 'a');
  
  auto result = propagator_.parseBaggage(large_baggage);
  
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.status().message(), testing::HasSubstr("exceeds maximum size limit"));
}

TEST_F(BaggagePropagatorTest, ParseBaggageTooManyMembers) {
  // Create baggage with too many members
  std::vector<std::string> members;
  for (size_t i = 0; i <= Constants::kMaxBaggageMembers; ++i) {
    members.push_back("key" + std::to_string(i) + "=value" + std::to_string(i));
  }
  std::string large_baggage = absl::StrJoin(members, ",");
  
  auto result = propagator_.parseBaggage(large_baggage);
  
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.status().message(), testing::HasSubstr("exceeds maximum member count"));
}

TEST_F(BaggagePropagatorTest, SerializeBaggageEmpty) {
  BaggageMap empty_map;
  
  auto result = propagator_.serializeBaggage(empty_map);
  
  EXPECT_EQ(result, "");
}

TEST_F(BaggagePropagatorTest, SerializeBaggageSimple) {
  BaggageMap baggage_map;
  BaggageMember member;
  member.key = "key1";
  member.value = "value1";
  baggage_map["key1"] = member;
  
  auto result = propagator_.serializeBaggage(baggage_map);
  
  EXPECT_EQ(result, "key1=value1");
}

TEST_F(BaggagePropagatorTest, SerializeBaggageWithProperties) {
  BaggageMap baggage_map;
  BaggageMember member;
  member.key = "key1";
  member.value = "value1";
  member.properties = {"property1", "property2"};
  baggage_map["key1"] = member;
  
  auto result = propagator_.serializeBaggage(baggage_map);
  
  EXPECT_EQ(result, "key1=value1;property1;property2");
}

TEST_F(BaggagePropagatorTest, InjectBaggageFromMap) {
  Tracing::TestTraceContextImpl trace_context{};
  
  BaggageMap baggage_map;
  BaggageMember member;
  member.key = "key1";
  member.value = "value1";
  baggage_map["key1"] = member;
  
  propagator_.injectBaggage(trace_context, baggage_map);
  
  auto result = propagator_.extractBaggage(trace_context);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), "key1=value1");
}

TEST_F(BaggagePropagatorTest, InjectBaggageFromString) {
  Tracing::TestTraceContextImpl trace_context{};
  
  propagator_.injectBaggage(trace_context, simple_baggage_);
  
  auto result = propagator_.extractBaggage(trace_context);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), simple_baggage_);
}

TEST_F(BaggagePropagatorTest, InjectBaggageEmpty) {
  Tracing::TestTraceContextImpl trace_context{};
  
  BaggageMap empty_map;
  propagator_.injectBaggage(trace_context, empty_map);
  
  auto result = propagator_.extractBaggage(trace_context);
  EXPECT_FALSE(result.has_value());
}

TEST_F(BaggagePropagatorTest, InjectBaggageTooLarge) {
  Tracing::TestTraceContextImpl trace_context{};
  
  // Create oversized baggage
  std::string large_baggage(Constants::kMaxBaggageSize + 1, 'a');
  propagator_.injectBaggage(trace_context, large_baggage);
  
  // Should not inject oversized baggage
  auto result = propagator_.extractBaggage(trace_context);
  EXPECT_FALSE(result.has_value());
}

TEST_F(BaggagePropagatorTest, RemoveBaggage) {
  Tracing::TestTraceContextImpl trace_context{{"baggage", simple_baggage_}};
  
  EXPECT_TRUE(propagator_.hasBaggage(trace_context));
  
  propagator_.removeBaggage(trace_context);
  
  EXPECT_FALSE(propagator_.hasBaggage(trace_context));
  auto result = propagator_.extractBaggage(trace_context);
  EXPECT_FALSE(result.has_value());
}

TEST_F(BaggagePropagatorTest, HasBaggage) {
  Tracing::TestTraceContextImpl trace_context_with{{"baggage", simple_baggage_}};
  Tracing::TestTraceContextImpl trace_context_without{};
  
  EXPECT_TRUE(propagator_.hasBaggage(trace_context_with));
  EXPECT_FALSE(propagator_.hasBaggage(trace_context_without));
}

TEST_F(BaggagePropagatorTest, GetBaggageValueSuccess) {
  Tracing::TestTraceContextImpl trace_context{{"baggage", multi_baggage_}};
  
  auto result1 = propagator_.getBaggageValue(trace_context, "key1");
  ASSERT_TRUE(result1.ok());
  EXPECT_EQ(result1.value(), "value1");
  
  auto result2 = propagator_.getBaggageValue(trace_context, "key2");
  ASSERT_TRUE(result2.ok());
  EXPECT_EQ(result2.value(), "value2");
}

TEST_F(BaggagePropagatorTest, GetBaggageValueNotFound) {
  Tracing::TestTraceContextImpl trace_context{{"baggage", simple_baggage_}};
  
  auto result = propagator_.getBaggageValue(trace_context, "nonexistent");
  
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.status().message(), testing::HasSubstr("not found"));
}

TEST_F(BaggagePropagatorTest, GetBaggageValueNoBaggage) {
  Tracing::TestTraceContextImpl trace_context{};
  
  auto result = propagator_.getBaggageValue(trace_context, "key1");
  
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.status().message(), testing::HasSubstr("No baggage found"));
}

TEST_F(BaggagePropagatorTest, SetBaggageValueNew) {
  Tracing::TestTraceContextImpl trace_context{};
  
  propagator_.setBaggageValue(trace_context, "newkey", "newvalue");
  
  auto result = propagator_.getBaggageValue(trace_context, "newkey");
  ASSERT_TRUE(result.ok());
  EXPECT_EQ(result.value(), "newvalue");
}

TEST_F(BaggagePropagatorTest, SetBaggageValueUpdate) {
  Tracing::TestTraceContextImpl trace_context{{"baggage", simple_baggage_}};
  
  propagator_.setBaggageValue(trace_context, "key1", "newvalue");
  
  auto result = propagator_.getBaggageValue(trace_context, "key1");
  ASSERT_TRUE(result.ok());
  EXPECT_EQ(result.value(), "newvalue");
}

TEST_F(BaggagePropagatorTest, SetBaggageValueAdd) {
  Tracing::TestTraceContextImpl trace_context{{"baggage", simple_baggage_}};
  
  propagator_.setBaggageValue(trace_context, "key2", "value2");
  
  // Should have both original and new key
  auto result1 = propagator_.getBaggageValue(trace_context, "key1");
  ASSERT_TRUE(result1.ok());
  EXPECT_EQ(result1.value(), "value1");
  
  auto result2 = propagator_.getBaggageValue(trace_context, "key2");
  ASSERT_TRUE(result2.ok());
  EXPECT_EQ(result2.value(), "value2");
}

TEST_F(BaggagePropagatorTest, SetBaggageValueInvalidKey) {
  Tracing::TestTraceContextImpl trace_context{};
  
  // Try to set with invalid key (contains invalid character)
  propagator_.setBaggageValue(trace_context, "key@invalid", "value");
  
  // Should silently ignore invalid key
  EXPECT_FALSE(propagator_.hasBaggage(trace_context));
}

TEST_F(BaggagePropagatorTest, SetBaggageValueKeyTooLong) {
  Tracing::TestTraceContextImpl trace_context{};
  
  // Create key that's too long
  std::string long_key(Constants::kMaxKeyLength + 1, 'k');
  propagator_.setBaggageValue(trace_context, long_key, "value");
  
  // Should silently ignore oversized key
  EXPECT_FALSE(propagator_.hasBaggage(trace_context));
}

TEST_F(BaggagePropagatorTest, SetBaggageValueTooLong) {
  Tracing::TestTraceContextImpl trace_context{};
  
  // Create value that's too long
  std::string long_value(Constants::kMaxValueLength + 1, 'v');
  propagator_.setBaggageValue(trace_context, "key", long_value);
  
  // Should silently ignore oversized value
  EXPECT_FALSE(propagator_.hasBaggage(trace_context));
}

TEST_F(BaggagePropagatorTest, RoundTripBaggage) {
  Tracing::TestTraceContextImpl trace_context{};
  
  BaggageMap original_map;
  BaggageMember member1;
  member1.key = "key1";
  member1.value = "value1";
  member1.properties = {"prop1"};
  original_map["key1"] = member1;
  
  BaggageMember member2;
  member2.key = "key2";
  member2.value = "value2";
  original_map["key2"] = member2;
  
  // Inject
  propagator_.injectBaggage(trace_context, original_map);
  
  // Extract and parse
  auto extracted = propagator_.extractBaggage(trace_context);
  ASSERT_TRUE(extracted.has_value());
  
  auto parsed = propagator_.parseBaggage(extracted.value());
  ASSERT_TRUE(parsed.ok());
  
  // Verify values
  ASSERT_EQ(parsed->size(), 2);
  EXPECT_EQ(parsed->at("key1").value, "value1");
  EXPECT_EQ(parsed->at("key1").properties.size(), 1);
  EXPECT_EQ(parsed->at("key1").properties[0], "prop1");
  EXPECT_EQ(parsed->at("key2").value, "value2");
  EXPECT_TRUE(parsed->at("key2").properties.empty());
}

// Test various edge cases for baggage key and value validation
TEST_F(BaggagePropagatorTest, BaggageKeyValidation) {
  // Valid keys
  std::vector<std::string> valid_keys = {
    "key", "key123", "key_name", "key-name", "key.name", "key*name"
  };
  
  for (const auto& key : valid_keys) {
    Tracing::TestTraceContextImpl trace_context{};
    propagator_.setBaggageValue(trace_context, key, "value");
    EXPECT_TRUE(propagator_.hasBaggage(trace_context)) << "Valid key should work: " << key;
  }
  
  // Invalid keys
  std::vector<std::string> invalid_keys = {
    "", "key@invalid", "key space", "key=invalid", "key,invalid", "key;invalid"
  };
  
  for (const auto& key : invalid_keys) {
    Tracing::TestTraceContextImpl trace_context{};
    propagator_.setBaggageValue(trace_context, key, "value");
    EXPECT_FALSE(propagator_.hasBaggage(trace_context)) << "Invalid key should be rejected: " << key;
  }
}

} // namespace
} // namespace Baggage
} // namespace W3c
} // namespace Propagators
} // namespace Extensions
} // namespace Envoy