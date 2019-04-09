/**
 * @file test_resource.cpp
 * @author L.-C. C.
 * @brief 
 * @version 0.1
 * @date 2019-03-18
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include "resource.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

class TestResource : public testing::Test {
public:
	mess::Resource<int, 2> resource;
};

TEST_F(TestResource, GetThenGet) {
	const auto res1 = resource.get();
	const auto res2 = resource.get();
	EXPECT_NE(&*res1, &*res2);
}

TEST_F(TestResource, GetDestroyThenGet) {
	const auto* ptr1 = &*resource.get();
	const auto res2 = resource.get();
	EXPECT_EQ(ptr1, &*res2);
}

// TEST_F(TestResource, PushThenGet) {
// 	resource.push(42);
// 	EXPECT_EQ(*resource.get(), 42);
// }