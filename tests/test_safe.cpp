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

	using SafeValueType = safe::Safe<ValueType, MockMutex>;
	using SafeRefType = safe::Safe<ValueType&, MockMutex>;
	using SafeConstValueType = safe::Safe<const ValueType, MockMutex>;

	using GuardType = SafeValueType::Guard;
	using ConstGuardType = SafeValueType::ConstGuard;
	using LockType = SafeValueType::Lock;
	using ConstLockType = SafeValueType::ConstLock;

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

TEST_F(SafeTest, MutexValueConstructor) {
	SafeValueType safeValue(mutex, value);

	EXPECT_EQ(safeValue.unsafe(), value);
	EXPECT_EQ(&static_cast<const SafeValueType&>(safeValue).unsafe(), &safeValue.unsafe());
	EXPECT_EQ(&safeValue.lockable, &mutex);
}

// TEST_F(SafeTest, MutexRefValueConstructor) {
// 	SafeValueType safeValue(mutex, value);

// 	EXPECT_EQ(safeValue.unsafe(), value);
// 	EXPECT_EQ(&static_cast<const SafeValueType&>(safeValue).unsafe(), &safeValue.unsafe());
// 	EXPECT_EQ(&safeValue.lockable, &mutex);
// }

TEST_F(SafeTest, MutexValueRefConstructor) {
	SafeRefType safeRef(mutex, value);

	EXPECT_EQ(&safeRef.unsafe(), &value);
	EXPECT_EQ(&static_cast<const SafeRefType&>(safeRef).unsafe(), &safeRef.unsafe());
	EXPECT_EQ(&safeRef.lockable, &mutex);
}

TEST_F(SafeTest, FunctionGuard) {
	SafeValueType safeValue(mutex, value);

	setLockUnlockCallExpectations();
	checkValueAccessors(safeValue.guard(), safeValue.unsafe());
}

TEST_F(SafeTest, ConstFunctionGuard) {

	const SafeValueType safeValue(mutex, value);
	
	setLockUnlockCallExpectations();
	checkValueAccessors(safeValue.guard(), safeValue.unsafe());
}

TEST_F(SafeTest, FunctionConstGuard) {
	SafeValueType safeValue(mutex, value);

	setLockUnlockCallExpectations();
	checkValueAccessors(safeValue.constGuard(), safeValue.unsafe());
}

TEST_F(SafeTest, FunctionLock) {
	SafeValueType safeValue(mutex, value);

	setLockUnlockCallExpectations();

	const auto lock = safeValue.lock();

	checkValueAccessors(lock, safeValue.unsafe());
	EXPECT_EQ(lock.lock.mutex(), &mutex);
	EXPECT_TRUE(lock.lock.owns_lock());
}

TEST_F(SafeTest, ConstFunctionLock) {
	const SafeValueType safeValue(mutex, value);

	setLockUnlockCallExpectations();

	const auto lock = safeValue.lock();

	checkValueAccessors(lock, safeValue.unsafe());
	EXPECT_EQ(lock.lock.mutex(), &mutex);
	EXPECT_TRUE(lock.lock.owns_lock());
}

TEST_F(SafeTest, FunctionConstLock) {
	SafeValueType safeValue(mutex, value);

	setLockUnlockCallExpectations();

	const auto lock = safeValue.constLock();

	checkValueAccessors(lock, safeValue.unsafe());
	EXPECT_EQ(lock.lock.mutex(), &mutex);
	EXPECT_TRUE(lock.lock.owns_lock());
}

TEST_F(LockTest, MutexValueConstructor) {
	setLockUnlockCallExpectations();

	const LockType lock(mutex, value);

	checkValueAccessors(lock, value);
	EXPECT_EQ(lock.lock.mutex(), &mutex);
	EXPECT_TRUE(lock.lock.owns_lock());
}

TEST_F(LockTest, MutexValueAdoptLockConstructor) {
	setUnlockOnlyCallExpectations();

	const LockType lock(mutex, value, std::adopt_lock_t());

	checkValueAccessors(lock, value);
	EXPECT_EQ(lock.lock.mutex(), &mutex);
	EXPECT_TRUE(lock.lock.owns_lock());
}

TEST_F(LockTest, MutexValueTryToLockConstructor) {
	setTryLockOnlyCallExpectations();

	const LockType lock(mutex, value, std::try_to_lock_t());

	checkValueAccessors(lock, value);
	EXPECT_EQ(lock.lock.mutex(), &mutex);
}

TEST_F(LockTest, MutexValueDeferLockConstructor) {
	const LockType lock(mutex, value, std::defer_lock_t());
	
	checkValueAccessors(lock, value);
	EXPECT_EQ(lock.lock.mutex(), &mutex);
	EXPECT_FALSE(lock.lock.owns_lock());
}

TEST_F(ConstLockTest, MutexValueConstructor) {
	setLockUnlockCallExpectations();

	const ConstLockType lock(mutex, value);

	checkValueAccessors(lock, value);
	EXPECT_EQ(lock.lock.mutex(), &mutex);
	EXPECT_TRUE(lock.lock.owns_lock());
}

TEST_F(ConstLockTest, MutexValueAdoptLockConstructor) {
	setUnlockOnlyCallExpectations();

	const ConstLockType lock(mutex, value, std::adopt_lock_t());

	checkValueAccessors(lock, value);
	EXPECT_EQ(lock.lock.mutex(), &mutex);
	EXPECT_TRUE(lock.lock.owns_lock());
}

TEST_F(ConstLockTest, MutexValueTryToLockConstructor) {
	setTryLockOnlyCallExpectations();

	const ConstLockType lock(mutex, value, std::try_to_lock_t());

	checkValueAccessors(lock, value);
	EXPECT_EQ(lock.lock.mutex(), &mutex);
}

TEST_F(ConstLockTest, MutexValueDeferLockConstructor) {
	const ConstLockType lock(mutex, value, std::defer_lock_t());
	
	checkValueAccessors(lock, value);
	EXPECT_EQ(lock.lock.mutex(), &mutex);
	EXPECT_FALSE(lock.lock.owns_lock());
}

TEST_F(GuardTest, MutexValueConstructor) {
	setLockUnlockCallExpectations();

	const GuardType guard(mutex, value);

	checkValueAccessors(guard, value);
}

TEST_F(GuardTest, MutexValueAdoptConstructor) {
	setUnlockOnlyCallExpectations();

	const GuardType guard(mutex, value, std::adopt_lock_t());

	checkValueAccessors(guard, value);
}

TEST_F(ConstGuardTest, MutexValueConstructor) {
	setLockUnlockCallExpectations();

	const ConstGuardType guard(mutex, value);

	checkValueAccessors(guard, value);
}

TEST_F(ConstGuardTest, MutexValueAdoptConstructor) {
	setUnlockOnlyCallExpectations();

	const ConstGuardType guard(mutex, value, std::adopt_lock_t());

	checkValueAccessors(guard, value);
}
