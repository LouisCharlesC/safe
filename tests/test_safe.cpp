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

#include <type_traits>
#include <vector>

class DummyLockable
{};
template<typename>
class DummyLock
{
public:
	DummyLock(DummyLockable& lockable) noexcept:
		m_lockable(lockable)
	{}

	DummyLockable* mutex() const {return &m_lockable;}

private:
	DummyLockable& m_lockable;
};

// class MockValue: public std::vector<int>
// {
// public:
// 	using std::vector<int>::vector;
//   MOCK_CONST_METHOD0(touch, void());
// };

class SafeTest : public testing::Test {
public:
	using SafeMutexRefValueRefType = safe::Safe<int&, DummyLockable&>;
	using SafeMutexRefConstValueRefType = safe::Safe<const int&, DummyLockable&>;
	using SafeMutexRefValueType = safe::Safe<int, DummyLockable&>;
	using SafeMutexRefConstValueType = safe::Safe<const int, DummyLockable&>;
	using SafeMutexValueRefType = safe::Safe<int&, DummyLockable>;
	using SafeMutexValueType = safe::Safe<int, DummyLockable>;

	using SharedAccessType = SafeMutexRefValueRefType::SharedAccess<DummyLock>;
	using AccessType = SafeMutexRefValueRefType::Access<DummyLock>;

	SafeTest():
		value(42)
	{}

	DummyLockable lockable;
	int value;
};

// // class AccessTest: public SafeTest {};
// // class LockTest: public SafeTest {};
// // class SharedGuardTest: public SafeTest {};
// // class GuardTest: public SafeTest {};

// // template<typename Lock>
// // void checkAccessorsAndTouchValue(Lock& lock, const MockValue& value)
// // {
// // 	EXPECT_EQ(&*lock, &value);
// // 	EXPECT_EQ(lock.operator->(), &value);
// // 	EXPECT_EQ(&*static_cast<const Lock&>(lock), &value);
// // 	EXPECT_EQ(static_cast<const Lock&>(lock).operator->(), &value);
// // 	value.touch();
// // }

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

void readmeWithSafeExample()
{
	std::mutex wrong_mutex;
	safe::Safe<std::vector<int>> safeVec; // <-- the right mutex is in here!
	{
		safe::Safe<std::vector<int>>::Access<std::lock_guard> vec(safeVec); // <-- right mutex: guaranteed!
		vec->push_back(42);
	}
	safeVec.unsafe().pop_back(); // <-- unprotected access: clearly expressed!
}

TEST_F(SafeTest, MutexRefValueRefConstructor) {
	SafeMutexRefValueRefType safe(lockable, value);

	EXPECT_EQ(&safe.unsafe(), &value);
	EXPECT_EQ(&static_cast<const SafeMutexRefValueRefType&>(safe).unsafe(), &safe.unsafe());
}
TEST_F(SafeTest, MutexRefConstValueRefConstructor) {
	SafeMutexRefConstValueRefType safe(lockable, value);

	EXPECT_EQ(&safe.unsafe(), &value);
	EXPECT_EQ(&static_cast<const SafeMutexRefConstValueRefType&>(safe).unsafe(), &safe.unsafe());
}
TEST_F(SafeTest, MutexRefValueConstructor) {
	SafeMutexRefValueType safe(lockable, value);

	EXPECT_EQ(safe.unsafe(), value);
	EXPECT_EQ(&static_cast<const SafeMutexRefValueType&>(safe).unsafe(), &safe.unsafe());
}
TEST_F(SafeTest, MutexRefConstValueConstructor) {
	SafeMutexRefConstValueType safe(lockable, value);

	EXPECT_EQ(safe.unsafe(), value);
	EXPECT_EQ(&static_cast<const SafeMutexRefConstValueType&>(safe).unsafe(), &safe.unsafe());
}
TEST_F(SafeTest, MutexValueRefConstructor) {
	SafeMutexValueRefType safe(safe::default_construct_lockable(), value);

	EXPECT_EQ(&safe.unsafe(), &value);
	EXPECT_EQ(&static_cast<const SafeMutexValueRefType&>(safe).unsafe(), &safe.unsafe());
}
TEST_F(SafeTest, MutexValueConstructor) {
	SafeMutexValueType safe(safe::default_construct_lockable(), value);

	EXPECT_EQ(safe.unsafe(), value);
	EXPECT_EQ(&static_cast<const SafeMutexValueType&>(safe).unsafe(), &safe.unsafe());
}
TEST_F(SafeTest, FunctionAccess) {
	SafeMutexRefValueRefType safe(lockable, value);
	
	auto access = safe.access<DummyLock>();
	
	EXPECT_EQ(&*access, &value);
	EXPECT_EQ(&*static_cast<const AccessType&>(access), &value);
	EXPECT_EQ(access.operator->(), &value);
	EXPECT_EQ(static_cast<const AccessType&>(access).operator->(), &value);
	EXPECT_EQ(access.getLock().mutex(), &lockable);
	EXPECT_EQ(static_cast<const AccessType&>(access).getLock().mutex(), &lockable);
}

// TEST_F(AccessTest, allo) {
// 	SafeMutexRefValueRefType safe(mutex, value);

// 	setLockTouchAndUnlockCallExpectations();
// 	SafeMutexRefValueRefType::AccessType<std::lock_guard> lock(safe);
// 	checkAccessorsAndTouchValue(lock, value);
// }

// TEST_F(SafeTest, allo2)
// {
// 	// using Ret = std::result_of<SafeMutexRefValueRefType::AccessType<std::lock_guard>::operator*()>::type;
// 	// static_assert(std::is_same<Ret, Value&>::value, "");
// }

// TEST_F(SafeTest, FunctionSharedLock) {
// 	SafeMutexRefValueRefType safe(mutex, value);

// 	setLockTouchAndUnlockCallExpectations();
// 	auto lock = safe.sharedLock();
// 	checkAccessorsAndTouchValue(lock, value);
// }
// TEST_F(SafeTest, ConstFunctionLock) {
// 	const SafeMutexRefValueRefType safe(mutex, value);

// 	setLockTouchAndUnlockCallExpectations();
// 	auto lock = safe.lock();
// 	checkAccessorsAndTouchValue(lock, value);
// }
// TEST_F(SafeTest, FunctionLock) {
// 	SafeMutexRefValueRefType safe(mutex, value);

// 	setLockTouchAndUnlockCallExpectations();
// 	auto lock = safe.lock();
// 	checkAccessorsAndTouchValue(lock, value);
// }

// TEST_F(SafeTest, FunctionSharedGuard) {
// 	SafeMutexRefValueRefType safe(mutex, value);

// 	setLockSharedTouchAndUnlockSharedCallExpectations();
// 	auto&& guard = safe.sharedGuard();
// 	checkAccessorsAndTouchValue(guard, safe.unsafe());
// }
// TEST_F(SafeTest, ConstFunctionGuard) {
// 	const SafeMutexRefValueRefType safe(mutex, value);
	
// 	setLockSharedTouchAndUnlockSharedCallExpectations();
// 	auto&& guard = safe.guard();
// 	checkAccessorsAndTouchValue(guard, safe.unsafe());
// }
// TEST_F(SafeTest, FunctionGuard) {
// 	SafeMutexRefValueRefType safe(mutex, value);

// 	setLockTouchAndUnlockCallExpectations();
// 	auto&& guard = safe.guard();
// 	checkAccessorsAndTouchValue(guard, safe.unsafe());
// }

// TEST_F(LockTest, SafeConstructor) {
// 	SafeMutexRefValueRefType safe(mutex, value);

// 	setLockTouchAndUnlockCallExpectations();
// 	LockType lock(safe);
// 	checkAccessorsAndTouchValue(lock, value);
// }

// TEST_F(SharedLockTest, SafeConstructor) {
// 	SafeMutexRefValueRefType safe(mutex, value);

// 	setLockTouchAndUnlockCallExpectations();
// 	SharedLockType lock(safe);
// 	checkAccessorsAndTouchValue(lock, value);
// }

// TEST_F(GuardTest, SafeConstructor) {
// 	SafeMutexRefValueRefType safe(mutex, value);

// 	setLockTouchAndUnlockCallExpectations();
// 	GuardType guard(safe);
// 	checkAccessorsAndTouchValue(guard, value);
// }
// TEST_F(GuardTest, LockConstructor) {
// 	SafeMutexRefValueRefType safe(mutex, value);

// 	setLockTouchAndUnlockCallExpectations();
// 	LockType lock(safe);
// 	GuardType guard(lock);
// 	checkAccessorsAndTouchValue(guard, value);
// }

// TEST_F(SharedGuardTest, SafeConstructor) {
// 	SafeMutexRefValueRefType safe(mutex, value);

// 	setLockSharedTouchAndUnlockSharedCallExpectations();
// 	SharedGuardType guard(safe);
// 	checkAccessorsAndTouchValue(guard, value);
// }
// // TEST_F(SharedGuardTest, SharedLockConstructor) {
// // 	SafeMutexRefValueRefType safe(mutex, value);

// // 	setLockSharedTouchAndUnlockSharedCallExpectations();
// // 	SharedLockType lock(safe);
// // 	SharedGuardType guard(lock);
// // 	checkAccessorsAndTouchValue(guard, value);
// // }
