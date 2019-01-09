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

#include <condition_variable>
#include <mutex>

class DummyLockable
{
public:
	MOCK_METHOD0(touch, void());

	MOCK_METHOD0(lock, void());
	MOCK_METHOD0(try_lock, bool());
	MOCK_METHOD0(unlock, void());
};
template<typename>
class DummyLock
{
public:
	DummyLock(DummyLockable& lockable) noexcept:
		lockable(lockable)
	{}

	DummyLockable& lockable;

private:
};

class SafeTest : public testing::Test {
public:
	using SafeLockableRefValueRefType = safe::Safe<int&, DummyLockable&>;
	using SafeLockableRefConstValueRefType = safe::Safe<const int&, DummyLockable&>;
	using SafeLockableRefValueType = safe::Safe<int, DummyLockable&>;
	using SafeLockableRefConstValueType = safe::Safe<const int, DummyLockable&>;
	using SafeLockableValueRefType = safe::Safe<int&, DummyLockable>;
	using SafeLockableValueType = safe::Safe<int, DummyLockable>;

	using SafeLockableRefValueRefConstAccessType = SafeLockableRefValueRefType::ConstAccess<DummyLock>;
	using SafeLockableRefValueRefAccessType = SafeLockableRefValueRefType::Access<DummyLock>;
	using SafeLockableValueConstAccessType = SafeLockableValueType::ConstAccess<DummyLock>;
	using SafeLockableValueAccessType = SafeLockableValueType::Access<DummyLock>;

	SafeTest():
		value(42)
	{}

	DummyLockable lockable;
	int value;
};

class AccessTest: public SafeTest {};
class UseCasesTest: public SafeTest {};

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
	// Convenience typedef
	using SafeVectorInt = safe::Safe<std::vector<int>, std::mutex&>;

	std::mutex wrong_mutex;
	std::mutex right_mutex;
	SafeVectorInt safeVec(right_mutex); // <-- value+mutex association!
	{
		safe::StdLockGuardAccess<SafeVectorInt> vec(safeVec); // <-- right mutex: guaranteed!
		vec->push_back(42); // access the vector using pointer semantics: * and ->
	}
	safeVec.unsafe().pop_back(); // <-- unprotected access: clearly expressed!
}

void readmeUniqueLockAccessIntoConditionVariableExample()
{
	safe::Safe<int> safe;
	safe::StdUniqueLockAccess<safe::Safe<int>> access(safe);
	std::condition_variable cv;
	cv.wait(access.lock);
}

TEST_F(SafeTest, LockableRefValueRefConstructor) {
	SafeLockableRefValueRefType safe(lockable, value);

	EXPECT_EQ(&safe.unsafe(), &value);
	EXPECT_EQ(&static_cast<const SafeLockableRefValueRefType&>(safe).unsafe(), &safe.unsafe());
}
TEST_F(SafeTest, LockableRefConstValueRefConstructor) {
	SafeLockableRefConstValueRefType safe(lockable, value);

	EXPECT_EQ(&safe.unsafe(), &value);
	EXPECT_EQ(&static_cast<const SafeLockableRefConstValueRefType&>(safe).unsafe(), &safe.unsafe());
}
TEST_F(SafeTest, LockableRefValueConstructor) {
	SafeLockableRefValueType safe(lockable, value);

	EXPECT_EQ(safe.unsafe(), value);
	EXPECT_EQ(&static_cast<const SafeLockableRefValueType&>(safe).unsafe(), &safe.unsafe());
}
TEST_F(SafeTest, LockableRefConstValueConstructor) {
	SafeLockableRefConstValueType safe(lockable, value);
#
	EXPECT_EQ(safe.unsafe(), value);
	EXPECT_EQ(&static_cast<const SafeLockableRefConstValueType&>(safe).unsafe(), &safe.unsafe());
}
TEST_F(SafeTest, DefaultLockableValueRefConstructor) {
	SafeLockableValueRefType safe(safe::default_construct_lockable, value);

	EXPECT_EQ(&safe.unsafe(), &value);
	EXPECT_EQ(&static_cast<const SafeLockableValueRefType&>(safe).unsafe(), &safe.unsafe());
}
TEST_F(SafeTest, DefaultLockableValueConstructor) {
	SafeLockableValueType safe(safe::default_construct_lockable, value);

	EXPECT_EQ(safe.unsafe(), value);
	EXPECT_EQ(&static_cast<const SafeLockableValueType&>(safe).unsafe(), &safe.unsafe());
}
TEST_F(SafeTest, DefaultLockableDefaultValueConstructor) {
	SafeLockableValueType safe;
}
TEST_F(AccessTest, SafeLockableRefValueRefConstAccess) {
	SafeLockableRefValueRefType safe(lockable, value);
	
	SafeLockableRefValueRefConstAccessType access(safe);
	
	EXPECT_EQ(&*access, &value);
	EXPECT_EQ(&*static_cast<const SafeLockableRefValueRefConstAccessType&>(access), &value);
	EXPECT_EQ(access.operator->(), &value);
	EXPECT_EQ(static_cast<const SafeLockableRefValueRefConstAccessType&>(access).operator->(), &value);
	EXPECT_EQ(&access.lock.lockable, &lockable);
	EXPECT_EQ(&static_cast<const SafeLockableRefValueRefConstAccessType&>(access).lock.lockable, &lockable);

}
TEST_F(SafeTest, SafeLockableValueConstAccess) {
	SafeLockableValueType safe(safe::default_construct_lockable, value);
	
	SafeLockableValueConstAccessType access(safe);
	
	EXPECT_EQ(&*access, &safe.unsafe());
	EXPECT_EQ(&*static_cast<const SafeLockableValueConstAccessType&>(access), &safe.unsafe());
	EXPECT_EQ(access.operator->(), &safe.unsafe());
	EXPECT_EQ(static_cast<const SafeLockableValueConstAccessType&>(access).operator->(), &safe.unsafe());
}
TEST_F(SafeTest, SafeLockableRefValueRefAccess) {
	SafeLockableRefValueRefType safe(lockable, value);
	
	SafeLockableRefValueRefAccessType access(safe);
	
	EXPECT_EQ(&*access, &value);
	EXPECT_EQ(&*static_cast<const SafeLockableRefValueRefAccessType&>(access), &value);
	EXPECT_EQ(access.operator->(), &value);
	EXPECT_EQ(static_cast<const SafeLockableRefValueRefAccessType&>(access).operator->(), &value);
	EXPECT_EQ(&access.lock.lockable, &lockable);
	EXPECT_EQ(&static_cast<const SafeLockableRefValueRefAccessType&>(access).lock.lockable, &lockable);
}
TEST_F(SafeTest, SafeLockableValueAccess) {
	SafeLockableValueType safe(safe::default_construct_lockable, value);
	
	SafeLockableValueAccessType access(safe);
	
	EXPECT_EQ(&*access, &safe.unsafe());
	EXPECT_EQ(&*static_cast<const SafeLockableValueAccessType&>(access), &safe.unsafe());
	EXPECT_EQ(access.operator->(), &safe.unsafe());
	EXPECT_EQ(static_cast<const SafeLockableValueAccessType&>(access).operator->(), &safe.unsafe());
	}

TEST_F(AccessTest, ReturnTypes) {
	static_assert(std::is_same<SafeLockableRefValueRefType::Access<DummyLock>::ConstPointerType, const int*>::value, "");
	static_assert(std::is_same<SafeLockableRefValueRefType::Access<DummyLock>::PointerType, int*>::value, "");
	static_assert(std::is_same<SafeLockableRefValueRefType::Access<DummyLock>::ConstReferenceType, const int&>::value, "");
	static_assert(std::is_same<SafeLockableRefValueRefType::Access<DummyLock>::ReferenceType, int&>::value, "");

	static_assert(std::is_same<SafeLockableRefValueRefType::ConstAccess<DummyLock>::ConstPointerType, const int*>::value, "");
	static_assert(std::is_same<SafeLockableRefValueRefType::ConstAccess<DummyLock>::PointerType, const int*>::value, "");
	static_assert(std::is_same<SafeLockableRefValueRefType::ConstAccess<DummyLock>::ConstReferenceType, const int&>::value, "");
	static_assert(std::is_same<SafeLockableRefValueRefType::ConstAccess<DummyLock>::ReferenceType, const int&>::value, "");

	static_assert(std::is_same<SafeLockableValueType::Access<DummyLock>::ConstPointerType, const int*>::value, "");
	static_assert(std::is_same<SafeLockableValueType::Access<DummyLock>::PointerType, int*>::value, "");
	static_assert(std::is_same<SafeLockableValueType::Access<DummyLock>::ConstReferenceType, const int&>::value, "");
	static_assert(std::is_same<SafeLockableValueType::Access<DummyLock>::ReferenceType, int&>::value, "");

	static_assert(std::is_same<SafeLockableValueType::ConstAccess<DummyLock>::ConstPointerType, const int*>::value, "");
	static_assert(std::is_same<SafeLockableValueType::ConstAccess<DummyLock>::PointerType, const int*>::value, "");
	static_assert(std::is_same<SafeLockableValueType::ConstAccess<DummyLock>::ConstReferenceType, const int&>::value, "");
	static_assert(std::is_same<SafeLockableValueType::ConstAccess<DummyLock>::ReferenceType, const int&>::value, "");
}
TEST_F(AccessTest, StdUniqueLockConstAccessToGuardLockConstAccess) {
	SafeLockableRefValueRefType safe(lockable, value);
	
	{
		testing::InSequence _;
		EXPECT_CALL(lockable, lock());
		EXPECT_CALL(lockable, touch());
		EXPECT_CALL(lockable, unlock());
		EXPECT_CALL(lockable, touch());
	}

	{
		safe::StdUniqueLockConstAccess<SafeLockableRefValueRefType> uniqueLockAccess(safe);
		{
			lockable.touch();
			safe::StdLockGuardConstAccess<SafeLockableRefValueRefType> lockGuardAccess(uniqueLockAccess);
		}
		lockable.touch();
	}
}
TEST_F(AccessTest, StdUniqueLockAccessToGuardLockConstAccess) {
	SafeLockableRefValueRefType safe(lockable, value);
	
	{
		testing::InSequence _;
		EXPECT_CALL(lockable, lock());
		EXPECT_CALL(lockable, touch());
		EXPECT_CALL(lockable, unlock());
		EXPECT_CALL(lockable, touch());
	}

	{
		safe::StdUniqueLockAccess<SafeLockableRefValueRefType> uniqueLockAccess(safe);
		{
			lockable.touch();
			safe::StdLockGuardConstAccess<SafeLockableRefValueRefType> lockGuardAccess(uniqueLockAccess);
		}
		lockable.touch();
	}
}
TEST_F(AccessTest, StdUniqueLockAccessToGuardLockAccess) {
	SafeLockableRefValueRefType safe(lockable, value);
	
	{
		testing::InSequence _;
		EXPECT_CALL(lockable, lock());
		EXPECT_CALL(lockable, touch());
		EXPECT_CALL(lockable, unlock());
		EXPECT_CALL(lockable, touch());
	}

	{
		safe::StdUniqueLockAccess<SafeLockableRefValueRefType> uniqueLockAccess(safe);
		{
			lockable.touch();
			safe::StdLockGuardAccess<SafeLockableRefValueRefType> lockGuardAccess(uniqueLockAccess);
		}
		lockable.touch();
	}
}