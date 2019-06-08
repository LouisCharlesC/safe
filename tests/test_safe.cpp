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

class TestSafe : public testing::Test {
public:
	TestSafe():
		safe(42)
	{}

	safe::Safe<int> safe;
};
class TestSafePtr : public testing::Test {
public:
	TestSafePtr():
		safe(42)
	{}

	safe::Safe<std::shared_ptr<int>> safe;
};

TEST_F(TestSafePtr, GetThenSet) {
	const auto value = safe.copy();
	safe.assign(43);
	const auto otherValue = safe.copy();

	EXPECT_EQ(*value, 42);
	EXPECT_EQ(*otherValue, 43);
}
TEST_F(TestSafe, GetThenSet) {
	const auto value = safe.copy();
	safe.assign(43);
	const auto otherValue = safe.copy();

	EXPECT_EQ(value, 42);
	EXPECT_EQ(otherValue, 43);
}

TEST_F(TestSafePtr, GetThenUpdate) {
	const auto value = safe.copy();
	**safe.writeAccess() = 43;
	const auto otherValue = safe.copy();

	EXPECT_EQ(*value, 42);
	EXPECT_EQ(*otherValue, 43);
}
TEST_F(TestSafe, GetThenUpdate) {
	const auto value = safe.copy();
	*safe.writeAccess() = 43;
	const auto otherValue = safe.copy();

	EXPECT_EQ(value, 42);
	EXPECT_EQ(otherValue, 43);
}

TEST_F(TestSafePtr, SetDoesNotReallocateIfUnique) {
	const auto ptr = safe.copy().get();
	safe.assign(43);
	const auto samePtr = safe.copy().get();

	EXPECT_EQ(ptr, samePtr);
}

TEST_F(TestSafePtr, UpdateDoesNotReallocateIfUnique) {
	const auto ptr = safe.copy().get();
	**safe.writeAccess() = 43;
	const auto samePtr = safe.copy().get();

	EXPECT_EQ(ptr, samePtr);
}

TEST_F(TestSafePtr, SetReallocatesIfNotUnique) {
	const auto value = safe.copy();
	const auto ptr = value.get();
	safe.assign(43);
	const auto otherPtr = safe.copy().get();

	EXPECT_EQ(*value, 42);
	EXPECT_NE(ptr, otherPtr);
	EXPECT_EQ(*otherPtr, 43);
}

TEST_F(TestSafePtr, UpdateReallocatesIfNotUnique) {
	const auto value = safe.copy();
	const auto ptr = value.get();
	**safe.writeAccess() = 43;
	const auto otherPtr = safe.copy().get();

	EXPECT_EQ(*value, 42);
	EXPECT_NE(ptr, otherPtr);
	EXPECT_EQ(*otherPtr, 43);
}

TEST_F(TestSafePtr, SeveralGetsAreEqual) {
	const auto ptr = safe.copy().get();
	const auto value = safe.copy();
	const auto samePtr = value.get();
	const auto alsoSamePtr = safe.copy().get();

	EXPECT_EQ(ptr, samePtr);
	EXPECT_EQ(ptr, alsoSamePtr);
}