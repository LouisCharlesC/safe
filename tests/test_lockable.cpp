/**
 * @file test_safe.cpp
 * @author L.-C. C.
 * @brief 
 * @version 0.1
 * @date 2018-11-24
 * 
 * @copyright Copyright (c) 2018
 * 
 */

#include "safe/lockable.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <condition_variable>
#include <mutex>
#include <string>

class DummyMutex
{
public:
	MOCK_METHOD0(lock, void());
	MOCK_METHOD0(try_lock, bool());
	MOCK_METHOD0(unlock, void());
};
template<typename>
class DummyLock
{
public:
	DummyLock(DummyMutex& mutex) noexcept:
		mutex(mutex)
	{}

	DummyMutex& mutex;

private:
};

class LockableTest : public testing::Test {
public:
	using SafeMutexRefValueRefType = safe::Lockable<int&, DummyMutex&>;
	using SafeMutexRefConstValueRefType = safe::Lockable<const int&, DummyMutex&>;
	using SafeMutexRefValueType = safe::Lockable<int, DummyMutex&>;
	using SafeMutexRefConstValueType = safe::Lockable<const int, DummyMutex&>;
	using SafeMutexValueRefType = safe::Lockable<int&, DummyMutex>;
	using SafeMutexValueType = safe::Lockable<int, DummyMutex>;

	using SafeMutexRefValueRefReadOnlyAccessType = safe::ReadAccess<SafeMutexRefValueRefType, DummyLock>;
	using SafeMutexRefValueRefReadWriteAccessType = safe::WriteAccess<SafeMutexRefValueRefType, DummyLock>;
	using SafeMutexValueReadOnlyAccessType = safe::ReadAccess<SafeMutexValueType, DummyLock>;
	using SafeMutexValueReadWriteAccessType = safe::WriteAccess<SafeMutexValueType, DummyLock>;

	LockableTest():
		value(42)
	{}

	DummyMutex mutex;
	int value;
};

TEST_F(LockableTest, MutexRefValueRefConstructor) {
	SafeMutexRefValueRefType lockable(mutex, value);

	EXPECT_EQ(&lockable.unsafe(), &value);
	EXPECT_EQ(&static_cast<const SafeMutexRefValueRefType&>(lockable).unsafe(), &lockable.unsafe());
}
TEST_F(LockableTest, MutexRefConstValueRefConstructor) {
	SafeMutexRefConstValueRefType lockable(mutex, value);

	EXPECT_EQ(&lockable.unsafe(), &value);
	EXPECT_EQ(&static_cast<const SafeMutexRefConstValueRefType&>(lockable).unsafe(), &lockable.unsafe());
}
TEST_F(LockableTest, MutexRefValueConstructor) {
	SafeMutexRefValueType lockable(mutex, value);

	EXPECT_EQ(lockable.unsafe(), value);
	EXPECT_EQ(&static_cast<const SafeMutexRefValueType&>(lockable).unsafe(), &lockable.unsafe());
}
TEST_F(LockableTest, MutexRefConstValueConstructor) {
	SafeMutexRefConstValueType lockable(mutex, value);
#
	EXPECT_EQ(lockable.unsafe(), value);
	EXPECT_EQ(&static_cast<const SafeMutexRefConstValueType&>(lockable).unsafe(), &lockable.unsafe());
}
TEST_F(LockableTest, DefaultMutexValueRefConstructor) {
	SafeMutexValueRefType lockable(safe::default_construct_mutex, value);

	EXPECT_EQ(&lockable.unsafe(), &value);
	EXPECT_EQ(&static_cast<const SafeMutexValueRefType&>(lockable).unsafe(), &lockable.unsafe());
}
TEST_F(LockableTest, DefaultMutexValueConstructor) {
	SafeMutexValueType lockable(safe::default_construct_mutex, value);

	EXPECT_EQ(lockable.unsafe(), value);
	EXPECT_EQ(&static_cast<const SafeMutexValueType&>(lockable).unsafe(), &lockable.unsafe());
}
TEST_F(LockableTest, DefaultMutexDefaultValueConstructor) {
	SafeMutexValueType lockable;
}
TEST_F(LockableTest, SafeMutexRefValueRefAccess) {
	SafeMutexRefValueRefType lockable(mutex, value);
	
	SafeMutexRefValueRefReadWriteAccessType access(lockable);
	
	EXPECT_EQ(&*access, &value);
	EXPECT_EQ(&*static_cast<const SafeMutexRefValueRefReadWriteAccessType&>(access), &value);
	EXPECT_EQ(access.operator->(), &value);
	EXPECT_EQ(static_cast<const SafeMutexRefValueRefReadWriteAccessType&>(access).operator->(), &value);
	EXPECT_EQ(&access.lock.mutex, &mutex);
	EXPECT_EQ(&static_cast<const SafeMutexRefValueRefReadWriteAccessType&>(access).lock.mutex, &mutex);
}
TEST_F(LockableTest, SafeMutexValueAccess) {
	SafeMutexValueType lockable(safe::default_construct_mutex, value);
	
	SafeMutexValueReadWriteAccessType access(lockable);
	
	EXPECT_EQ(&*access, &lockable.unsafe());
	EXPECT_EQ(&*static_cast<const SafeMutexValueReadWriteAccessType&>(access), &lockable.unsafe());
	EXPECT_EQ(access.operator->(), &lockable.unsafe());
	EXPECT_EQ(static_cast<const SafeMutexValueReadWriteAccessType&>(access).operator->(), &lockable.unsafe());
}
TEST_F(LockableTest, ConstSafeMutexRefValueRefReadOnlyAccess) {
	const SafeMutexRefValueRefType lockable(mutex, value);
	
	SafeMutexRefValueRefReadOnlyAccessType access(lockable);
	
	EXPECT_EQ(&*access, &value);
	EXPECT_EQ(&*static_cast<const SafeMutexRefValueRefReadOnlyAccessType&>(access), &value);
	EXPECT_EQ(access.operator->(), &value);
	EXPECT_EQ(static_cast<const SafeMutexRefValueRefReadOnlyAccessType&>(access).operator->(), &value);
	EXPECT_EQ(&access.lock.mutex, &mutex);
	EXPECT_EQ(&static_cast<const SafeMutexRefValueRefReadOnlyAccessType&>(access).lock.mutex, &mutex);
}
TEST_F(LockableTest, ConstSafeMutexValueReadOnlyAccess) {
	const SafeMutexValueType lockable(safe::default_construct_mutex, value);
	
	SafeMutexValueReadOnlyAccessType access(lockable);
	
	EXPECT_EQ(&*access, &lockable.unsafe());
	EXPECT_EQ(&*static_cast<const SafeMutexValueReadOnlyAccessType&>(access), &lockable.unsafe());
	EXPECT_EQ(access.operator->(), &lockable.unsafe());
	EXPECT_EQ(static_cast<const SafeMutexValueReadOnlyAccessType&>(access).operator->(), &lockable.unsafe());
}

TEST_F(LockableTest, ReturnTypes) {
	static_assert(std::is_same<safe::WriteAccess<SafeMutexRefValueRefType>::ConstPointerType, const int*>::value, "");
	static_assert(std::is_same<safe::WriteAccess<SafeMutexRefValueRefType>::PointerType, int*>::value, "");
	static_assert(std::is_same<safe::WriteAccess<SafeMutexRefValueRefType>::ConstReferenceType, const int&>::value, "");
	static_assert(std::is_same<safe::WriteAccess<SafeMutexRefValueRefType>::ReferenceType, int&>::value, "");

	static_assert(std::is_same<safe::ReadAccess<SafeMutexRefValueRefType>::ConstPointerType, const int*>::value, "");
	static_assert(std::is_same<safe::ReadAccess<SafeMutexRefValueRefType>::PointerType, const int*>::value, "");
	static_assert(std::is_same<safe::ReadAccess<SafeMutexRefValueRefType>::ConstReferenceType, const int&>::value, "");
	static_assert(std::is_same<safe::ReadAccess<SafeMutexRefValueRefType>::ReferenceType, const int&>::value, "");

	static_assert(std::is_same<safe::WriteAccess<SafeMutexValueType>::ConstPointerType, const int*>::value, "");
	static_assert(std::is_same<safe::WriteAccess<SafeMutexValueType>::PointerType, int*>::value, "");
	static_assert(std::is_same<safe::WriteAccess<SafeMutexValueType>::ConstReferenceType, const int&>::value, "");
	static_assert(std::is_same<safe::WriteAccess<SafeMutexValueType>::ReferenceType, int&>::value, "");

	static_assert(std::is_same<safe::ReadAccess<SafeMutexValueType>::ConstPointerType, const int*>::value, "");
	static_assert(std::is_same<safe::ReadAccess<SafeMutexValueType>::PointerType, const int*>::value, "");
	static_assert(std::is_same<safe::ReadAccess<SafeMutexValueType>::ConstReferenceType, const int&>::value, "");
	static_assert(std::is_same<safe::ReadAccess<SafeMutexValueType>::ReferenceType, const int&>::value, "");
}