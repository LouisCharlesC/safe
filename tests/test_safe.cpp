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

class MockMutex : public std::mutex {
public:

  MOCK_METHOD0(lock, void());
  MOCK_METHOD0(try_lock, bool());
  MOCK_METHOD0(unlock, void());
};

class SafeTest : public testing::Test {
public:
	using ValueType = int;

	using SafeMutexRefValueType = safe::Safe<ValueType, MockMutex&>;
	using SafeMutexValueType = safe::Safe<ValueType, MockMutex>;
	using SafeMutexRefValueRefType = safe::Safe<ValueType&, MockMutex&>;
	using SafeMutexRefConstValueType = safe::Safe<const ValueType, MockMutex&>;

	using ValueGuardType = SafeMutexRefValueType::Guard;
	using ValueConstGuardType = SafeMutexRefValueType::SharedGuard;
	using ValueLockType = SafeMutexRefValueType::Lock;
	using ValueConstLockType = SafeMutexRefValueType::SharedLock;

	void setUnlockOnlyCallExpectations()
	{
		testing::InSequence unlock;
		EXPECT_CALL(mutex, unlock());
	}
	void setLockUnlockCallExpectations()
	{
		testing::InSequence lockUnlock;
		EXPECT_CALL(mutex, lock());
		EXPECT_CALL(mutex, unlock());
	}
	void setTryLockOnlyCallExpectations()
	{
		testing::InSequence lockUnlock;
		EXPECT_CALL(mutex, try_lock());
	}

	MockMutex mutex;
	ValueType value = 42;
};

class LockTest: public SafeTest {};
class ConstLockTest: public SafeTest {};
class GuardTest: public SafeTest {};
class ConstGuardTest: public SafeTest {};

template<typename ConstOrNotGuardOrLock>
void checkValueAccessors(ConstOrNotGuardOrLock&& guardOrLock, const int& value)
{
	EXPECT_EQ(&*guardOrLock, &value);
	EXPECT_EQ(guardOrLock.operator->(), &value);
}

TEST_F(SafeTest, MutexRefValueConstructor) {
	SafeMutexRefValueType safeValue(mutex, value);

	EXPECT_EQ(safeValue.unsafe(), value);
	EXPECT_EQ(&static_cast<const SafeMutexRefValueType&>(safeValue).unsafe(), &safeValue.unsafe());
	EXPECT_EQ(&safeValue.lockable, &mutex);
}

TEST_F(SafeTest, MutexValueConstructor) {
	SafeMutexValueType safeValue(safe::default_construct_lockable(), value);

	EXPECT_EQ(safeValue.unsafe(), value);
	EXPECT_EQ(&static_cast<const SafeMutexValueType&>(safeValue).unsafe(), &safeValue.unsafe());
}

TEST_F(SafeTest, MutexRefValueRefConstructor) {
	SafeMutexRefValueRefType safeRef(mutex, value);

	EXPECT_EQ(&safeRef.unsafe(), &value);
	EXPECT_EQ(&static_cast<const SafeMutexRefValueRefType&>(safeRef).unsafe(), &safeRef.unsafe());
	EXPECT_EQ(&safeRef.lockable, &mutex);
}

TEST_F(SafeTest, FunctionGuard) {
	SafeMutexRefValueType safeValue(mutex, value);

	setLockUnlockCallExpectations();
	checkValueAccessors(safeValue.guard(), safeValue.unsafe());
}

TEST_F(SafeTest, ConstFunctionGuard) {

	const SafeMutexRefValueType safeValue(mutex, value);
	
	setLockUnlockCallExpectations();
	checkValueAccessors(safeValue.guard(), safeValue.unsafe());
}

TEST_F(SafeTest, FunctionConstGuard) {
	SafeMutexRefValueType safeValue(mutex, value);

	setLockUnlockCallExpectations();
	checkValueAccessors(safeValue.sharedGuard(), safeValue.unsafe());
}

TEST_F(SafeTest, FunctionLock) {
	SafeMutexRefValueType safeValue(mutex, value);

	setLockUnlockCallExpectations();

	const auto lock = safeValue.lock();

	checkValueAccessors(lock, safeValue.unsafe());
	EXPECT_EQ(lock.lock.mutex(), &mutex);
	EXPECT_TRUE(lock.lock.owns_lock());
}

TEST_F(SafeTest, ConstFunctionLock) {
	const SafeMutexRefValueType safeValue(mutex, value);

	setLockUnlockCallExpectations();

	const auto lock = safeValue.lock();

	checkValueAccessors(lock, safeValue.unsafe());
	EXPECT_EQ(lock.lock.mutex(), &mutex);
	EXPECT_TRUE(lock.lock.owns_lock());
}

TEST_F(SafeTest, FunctionConstLock) {
	SafeMutexRefValueType safeValue(mutex, value);

	setLockUnlockCallExpectations();

	const auto lock = safeValue.sharedLock();

	checkValueAccessors(lock, safeValue.unsafe());
	EXPECT_EQ(lock.lock.mutex(), &mutex);
	EXPECT_TRUE(lock.lock.owns_lock());
}

TEST_F(LockTest, MutexValueConstructor) {
	setLockUnlockCallExpectations();

	const ValueLockType lock(mutex, value);

	checkValueAccessors(lock, value);
	EXPECT_EQ(lock.lock.mutex(), &mutex);
	EXPECT_TRUE(lock.lock.owns_lock());
}

TEST_F(LockTest, MutexValueAdoptLockConstructor) {
	setUnlockOnlyCallExpectations();

	const ValueLockType lock(mutex, value, std::adopt_lock_t());

	checkValueAccessors(lock, value);
	EXPECT_EQ(lock.lock.mutex(), &mutex);
	EXPECT_TRUE(lock.lock.owns_lock());
}

TEST_F(LockTest, MutexValueTryToLockConstructor) {
	setTryLockOnlyCallExpectations();

	const ValueLockType lock(mutex, value, std::try_to_lock_t());

	checkValueAccessors(lock, value);
	EXPECT_EQ(lock.lock.mutex(), &mutex);
}

TEST_F(LockTest, MutexValueDeferLockConstructor) {
	const ValueLockType lock(mutex, value, std::defer_lock_t());
	
	checkValueAccessors(lock, value);
	EXPECT_EQ(lock.lock.mutex(), &mutex);
	EXPECT_FALSE(lock.lock.owns_lock());
}

TEST_F(ConstLockTest, MutexValueConstructor) {
	setLockUnlockCallExpectations();

	const ValueConstLockType lock(mutex, value);

	checkValueAccessors(lock, value);
	EXPECT_EQ(lock.lock.mutex(), &mutex);
	EXPECT_TRUE(lock.lock.owns_lock());
}

TEST_F(ConstLockTest, MutexValueAdoptLockConstructor) {
	setUnlockOnlyCallExpectations();

	const ValueConstLockType lock(mutex, value, std::adopt_lock_t());

	checkValueAccessors(lock, value);
	EXPECT_EQ(lock.lock.mutex(), &mutex);
	EXPECT_TRUE(lock.lock.owns_lock());
}

TEST_F(ConstLockTest, MutexValueTryToLockConstructor) {
	setTryLockOnlyCallExpectations();

	const ValueConstLockType lock(mutex, value, std::try_to_lock_t());

	checkValueAccessors(lock, value);
	EXPECT_EQ(lock.lock.mutex(), &mutex);
}

TEST_F(ConstLockTest, MutexValueDeferLockConstructor) {
	const ValueConstLockType lock(mutex, value, std::defer_lock_t());
	
	checkValueAccessors(lock, value);
	EXPECT_EQ(lock.lock.mutex(), &mutex);
	EXPECT_FALSE(lock.lock.owns_lock());
}

TEST_F(GuardTest, MutexValueConstructor) {
	setLockUnlockCallExpectations();

	const ValueGuardType guard(mutex, value);

	checkValueAccessors(guard, value);
}

TEST_F(GuardTest, MutexValueAdoptConstructor) {
	setUnlockOnlyCallExpectations();

	const ValueGuardType guard(mutex, value, std::adopt_lock_t());

	checkValueAccessors(guard, value);
}

TEST_F(ConstGuardTest, MutexValueConstructor) {
	setLockUnlockCallExpectations();

	const ValueConstGuardType guard(mutex, value);

	checkValueAccessors(guard, value);
}

TEST_F(ConstGuardTest, MutexValueAdoptConstructor) {
	setUnlockOnlyCallExpectations();

	const ValueConstGuardType guard(mutex, value, std::adopt_lock_t());

	checkValueAccessors(guard, value);
}
