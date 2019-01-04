//============================================================================
// Name        : localSafe.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "safe.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <vector>

class MockMutex : public std::mutex
{
public:
  MOCK_METHOD0(lock, void());
  MOCK_METHOD0(try_lock, bool());
  MOCK_METHOD0(unlock, void());

  MOCK_METHOD0(lock_shared, void());
  MOCK_METHOD0(try_lock_shared, bool());
  MOCK_METHOD0(unlock_shared, void());
};

class MockValue: public std::vector<int>
{
public:
	using std::vector<int>::vector;
  MOCK_CONST_METHOD0(touch, void());
};

class SafeTest : public testing::Test {
public:
	using SafeMutexRefValueRefType = safe::Safe<MockValue&, MockMutex&>;
	using SafeMutexRefConstValueRefType = safe::Safe<const MockValue&, MockMutex&>;
	using SafeMutexRefValueType = safe::Safe<MockValue, MockMutex&>;
	using SafeMutexValueRefType = safe::Safe<MockValue&, MockMutex>;
	using SafeMutexValueType = safe::Safe<MockValue, MockMutex>;

	using GuardType = SafeMutexRefValueRefType::Guard;
	using SharedGuardType = SafeMutexRefValueRefType::SharedGuard;
	using LockType = SafeMutexRefValueRefType::Lock;
	using SharedLockType = SafeMutexRefValueRefType::SharedLock;

	SafeTest():
		value(1ul, 42)
	{}

	void setTouchOnlyCallExpectations()
	{
		EXPECT_CALL(value, touch());
	}
	void setTouchAndUnlockCallExpectations()
	{
		testing::InSequence _;
		EXPECT_CALL(value, touch());
		EXPECT_CALL(mutex, unlock());
	}
	void setTouchAndUnlockSharedCallExpectations()
	{
		testing::InSequence _;
		EXPECT_CALL(value, touch());
		EXPECT_CALL(mutex, unlock_shared());
	}
	void setLockTouchAndUnlockCallExpectations()
	{
		testing::InSequence _;
		EXPECT_CALL(mutex, lock());
		EXPECT_CALL(value, touch());
		EXPECT_CALL(mutex, unlock());
	}
	void setLockSharedTouchAndUnlockSharedCallExpectations()
	{
		testing::InSequence _;
		EXPECT_CALL(mutex, lock_shared());
		EXPECT_CALL(value, touch());
		EXPECT_CALL(mutex, unlock_shared());
	}
	void setTryLockAndTouchCallExpectations()
	{
		testing::InSequence _;
		EXPECT_CALL(mutex, try_lock());
		EXPECT_CALL(value, touch());
	}
	void setTryLockSharedAndTouchCallExpectations()
	{
		testing::InSequence _;
		EXPECT_CALL(mutex, try_lock_shared());
		EXPECT_CALL(value, touch());
	}

	MockMutex mutex;
	MockValue value;
};

class SharedLockTest: public SafeTest {};
class LockTest: public SafeTest {};
class SharedGuardTest: public SafeTest {};
class GuardTest: public SafeTest {};

template<typename Lock>
void checkAccessorsAndTouchValue(Lock& lock, const MockValue& value)
{
	EXPECT_EQ(&*lock, &value);
	EXPECT_EQ(lock.operator->(), &value);
	EXPECT_EQ(&*static_cast<const Lock&>(lock), &value);
	EXPECT_EQ(static_cast<const Lock&>(lock).operator->(), &value);
	value.touch();
}

void readmeWithoutSafeExample()
{
	std::mutex wrong_mutex;
	std::mutex right_mutex;
	std::vector<int> vec;
	{
		std::lock_guard<std::mutex> lock(wrong_mutex); // <-- wrong mutex, but how could you tell ?
		vec.push_back(42);
	}
	vec.pop_back(); // <-- unprotected access, is this intended ?
}

void readmeWitSafeExample()
{
	std::mutex wrong_mutex;
	safe::Safe<std::vector<int>> safeVec; // <-- the right mutex is in here!
	{
		safe::Safe<std::vector<int>>::Guard vec(safeVec); // <-- right mutex: guaranteed!
		vec->push_back(42);
	}
	safeVec.unsafe().pop_back(); // <-- unprotected access: clearly expressed!
}

TEST_F(SafeTest, MutexRefValueRefConstructor) {
	SafeMutexRefValueRefType safe(mutex, value);

	EXPECT_EQ(&safe.unsafe(), &value);
	EXPECT_EQ(&static_cast<const SafeMutexRefValueRefType&>(safe).unsafe(), &safe.unsafe());
}
TEST_F(SafeTest, MutexRefConstValueRefConstructor) {
	SafeMutexRefConstValueRefType safe(mutex, value);

	EXPECT_EQ(&safe.unsafe(), &value);
	EXPECT_EQ(&static_cast<const SafeMutexRefConstValueRefType&>(safe).unsafe(), &safe.unsafe());
}
TEST_F(SafeTest, MutexRefValueConstructor) {
	SafeMutexRefValueType safe(mutex, value.cbegin(), value.cend());

	EXPECT_EQ(safe.unsafe(), value);
	EXPECT_EQ(&static_cast<const SafeMutexRefValueType&>(safe).unsafe(), &safe.unsafe());
}
TEST_F(SafeTest, MutexValueRefConstructor) {
	SafeMutexValueRefType safe(safe::default_construct_lockable(), value);

	EXPECT_EQ(&safe.unsafe(), &value);
	EXPECT_EQ(&static_cast<const SafeMutexValueRefType&>(safe).unsafe(), &safe.unsafe());
}
TEST_F(SafeTest, MutexValueConstructor) {
	SafeMutexValueType safe(safe::default_construct_lockable(), value.cbegin(), value.cend());

	EXPECT_EQ(safe.unsafe(), value);
	EXPECT_EQ(&static_cast<const SafeMutexValueType&>(safe).unsafe(), &safe.unsafe());
}

TEST_F(SafeTest, FunctionSharedLock) {
	SafeMutexRefValueRefType safe(mutex, value);

	setLockTouchAndUnlockCallExpectations();
	auto lock = safe.sharedLock();
	checkAccessorsAndTouchValue(lock, value);
}
TEST_F(SafeTest, ConstFunctionLock) {
	const SafeMutexRefValueRefType safe(mutex, value);

	setLockTouchAndUnlockCallExpectations();
	auto lock = safe.lock();
	checkAccessorsAndTouchValue(lock, value);
}
TEST_F(SafeTest, FunctionLock) {
	SafeMutexRefValueRefType safe(mutex, value);

	setLockTouchAndUnlockCallExpectations();
	auto lock = safe.lock();
	checkAccessorsAndTouchValue(lock, value);
}

TEST_F(SafeTest, FunctionSharedGuard) {
	SafeMutexRefValueRefType safe(mutex, value);

	setLockSharedTouchAndUnlockSharedCallExpectations();
	auto&& guard = safe.sharedGuard();
	checkAccessorsAndTouchValue(guard, safe.unsafe());
}
TEST_F(SafeTest, ConstFunctionGuard) {
	const SafeMutexRefValueRefType safe(mutex, value);
	
	setLockSharedTouchAndUnlockSharedCallExpectations();
	auto&& guard = safe.guard();
	checkAccessorsAndTouchValue(guard, safe.unsafe());
}
TEST_F(SafeTest, FunctionGuard) {
	SafeMutexRefValueRefType safe(mutex, value);

	setLockTouchAndUnlockCallExpectations();
	auto&& guard = safe.guard();
	checkAccessorsAndTouchValue(guard, safe.unsafe());
}

TEST_F(LockTest, SafeConstructor) {
	SafeMutexRefValueRefType safe(mutex, value);

	setLockTouchAndUnlockCallExpectations();
	LockType lock(safe);
	checkAccessorsAndTouchValue(lock, value);
}

TEST_F(SharedLockTest, SafeConstructor) {
	SafeMutexRefValueRefType safe(mutex, value);

	setLockTouchAndUnlockCallExpectations();
	SharedLockType lock(safe);
	checkAccessorsAndTouchValue(lock, value);
}

TEST_F(GuardTest, SafeConstructor) {
	SafeMutexRefValueRefType safe(mutex, value);

	setLockTouchAndUnlockCallExpectations();
	GuardType guard(safe);
	checkAccessorsAndTouchValue(guard, value);
}
TEST_F(GuardTest, LockConstructor) {
	SafeMutexRefValueRefType safe(mutex, value);

	setLockTouchAndUnlockCallExpectations();
	LockType lock(safe);
	GuardType guard(lock);
	checkAccessorsAndTouchValue(guard, value);
}

TEST_F(SharedGuardTest, SafeConstructor) {
	SafeMutexRefValueRefType safe(mutex, value);

	setLockSharedTouchAndUnlockSharedCallExpectations();
	SharedGuardType guard(safe);
	checkAccessorsAndTouchValue(guard, value);
}
// TEST_F(SharedGuardTest, SharedLockConstructor) {
// 	SafeMutexRefValueRefType safe(mutex, value);

// 	setLockSharedTouchAndUnlockSharedCallExpectations();
// 	SharedLockType lock(safe);
// 	SharedGuardType guard(lock);
// 	checkAccessorsAndTouchValue(guard, value);
// }
