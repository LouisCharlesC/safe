/**
 * @file test_state.cpp
 * @author L.-C. C.
 * @brief 
 * @version 0.1
 * @date 2019-03-18
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include "safe/safe.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

class TestSafe : public testing::Test {
public:
	TestSafe():
		safe(42)
	{}

	safe::Safe<std::shared_ptr<int>> safe;
};

TEST_F(TestSafe, GetThenSet) {
	const auto value = safe.copy();
	safe = 43;
	const auto otherValue = safe.copy();

	EXPECT_EQ(*value, 42);
	EXPECT_EQ(*otherValue, 43);
}

TEST_F(TestSafe, GetThenUpdate) {
	const auto value = safe.copy();
	**safe.writeAccess() = 43;
	const auto otherValue = safe.copy();

	EXPECT_EQ(*value, 42);
	EXPECT_EQ(*otherValue, 43);
}

TEST_F(TestSafe, SetDoesNotReallocateIfUnique) {
	const auto ptr = safe.copy().get();
	safe = 43;
	const auto samePtr = safe.copy().get();

	EXPECT_EQ(ptr, samePtr);
}

TEST_F(TestSafe, UpdateDoesNotReallocateIfUnique) {
	const auto ptr = safe.copy().get();
	**safe.writeAccess() = 43;
	const auto samePtr = safe.copy().get();

	EXPECT_EQ(ptr, samePtr);
}

TEST_F(TestSafe, SetReallocatesIfNotUnique) {
	const auto value = safe.copy();
	const auto ptr = value.get();
	safe = 43;
	const auto otherPtr = safe.copy().get();

	EXPECT_EQ(*value, 42);
	EXPECT_NE(ptr, otherPtr);
	EXPECT_EQ(*otherPtr, 43);
}

TEST_F(TestSafe, UpdateReallocatesIfNotUnique) {
	const auto value = safe.copy();
	const auto ptr = value.get();
	**safe.writeAccess() = 43;
	const auto otherPtr = safe.copy().get();

	EXPECT_EQ(*value, 42);
	EXPECT_NE(ptr, otherPtr);
	EXPECT_EQ(*otherPtr, 43);
}

TEST_F(TestSafe, SeveralGetsAreEqual) {
	const auto ptr = safe.copy().get();
	const auto value = safe.copy();
	const auto samePtr = value.get();
	const auto alsoSamePtr = safe.copy().get();

	EXPECT_EQ(ptr, samePtr);
	EXPECT_EQ(ptr, alsoSamePtr);
}