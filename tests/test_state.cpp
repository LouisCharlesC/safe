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

#include "state.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

class TestState : public testing::Test {
public:
	TestState():
		state(42)
	{}

	mess::State<std::shared_ptr<int>> state;
};

TEST_F(TestState, GetThenSet) {
	const auto value = state.get();
	state.set(43);
	const auto otherValue = state.get();

	EXPECT_EQ(*value, 42);
	EXPECT_EQ(*otherValue, 43);
}

TEST_F(TestState, GetThenUpdate) {
	const auto value = state.get();
	**state.update() = 43;
	const auto otherValue = state.get();

	EXPECT_EQ(*value, 42);
	EXPECT_EQ(*otherValue, 43);
}

TEST_F(TestState, SetDoesNotReallocateIfUnique) {
	const auto ptr = state.get().get();
	state.set(43);
	const auto samePtr = state.get().get();

	EXPECT_EQ(ptr, samePtr);
}

TEST_F(TestState, UpdateDoesNotReallocateIfUnique) {
	const auto ptr = state.get().get();
	**state.update() = 43;
	const auto samePtr = state.get().get();

	EXPECT_EQ(ptr, samePtr);
}

TEST_F(TestState, SetReallocatesIfNotUnique) {
	const auto value = state.get();
	const auto ptr = value.get();
	state.set(43);
	const auto otherPtr = state.get().get();

	EXPECT_EQ(*value, 42);
	EXPECT_NE(ptr, otherPtr);
	EXPECT_EQ(*otherPtr, 43);
}

TEST_F(TestState, UpdateReallocatesIfNotUnique) {
	const auto value = state.get();
	const auto ptr = value.get();
	**state.update() = 43;
	const auto otherPtr = state.get().get();

	EXPECT_EQ(*value, 42);
	EXPECT_NE(ptr, otherPtr);
	EXPECT_EQ(*otherPtr, 43);
}

TEST_F(TestState, SeveralGetsAreEqual) {
	const auto ptr = state.get().get();
	const auto value = state.get();
	const auto samePtr = value.get();
	const auto alsoSamePtr = state.get().get();

	EXPECT_EQ(ptr, samePtr);
	EXPECT_EQ(ptr, alsoSamePtr);
}