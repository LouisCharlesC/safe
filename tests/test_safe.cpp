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

	using SafeLockableRefValueRefSharedAccessType = SafeLockableRefValueRefType::Access<DummyLock, safe::ReadOnly>;
	using SafeLockableRefValueRefAccessType = SafeLockableRefValueRefType::Access<DummyLock>;
	using SafeLockableValueSharedAccessType = SafeLockableValueType::Access<DummyLock, safe::ReadOnly>;
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
std::mutex frontEndMutex;
std::mutex backEndMutex;
int count; // <-- do I need to lock a mutex to safely access this variable ?
{
	std::lock_guard<std::mutex> lock(frontEndMutex); // <-- is this the right mutex ?
	++count;
}
--count; // <-- unprotected access, is this intended ?
}

void readmeWithSafeExample()
{
std::mutex frontEndMutex;
safe::Safe<int> safeCount; // <-- value and mutex packaged together!
{
	safe::LockGuard<safe::Safe<int>> count(safeCount); // <-- right mutex: guaranteed!
	++*count; // access the vector using pointer semantics: * and ->
}
--safeCount.unsafe(); // <-- unprotected access: clearly expressed!
}

void readmeTag()
{
std::mutex aMutex;

safe::Safe<int, std::mutex> bothDefault; // value and lockable are default constructed, ok
safe::Safe<int, std::mutex&> noDefault(aMutex, 42); // value and lockable initialized, ok
safe::Safe<int, std::mutex&> valueDefault(aMutex); // value is default constructed, and lockable is initialized, ok
safe::Safe<int, std::mutex> lockableDefault(safe::default_construct_lockable, 42); // value is initialized to 42, and mutex is default constructed: need the safe::default_construct_lockable tag!
}

void readmeUniqueLockToLockGuard()
{
safe::Safe<int> safeValue;

safe::UniqueLock<safe::Safe<int>> uniqueLockAccess(safeValue);
safe::LockGuard<safe::Safe<int>> lockGuardAccess(*uniqueLockAccess, *uniqueLockAccess.lock.release(), std::adopt_lock);
}

void readmeUniqueLockAccessIntoConditionVariableExample()
{
std::condition_variable cv;
safe::Safe<int> safeValue;

safe::UniqueLock<safe::Safe<int>> access(safeValue);
cv.wait(access.lock);
}

void readmeReturnStdLockGuard()
{
class MultithreadCount
{
public:
	void increment()
 	{
  		++*safe::LockGuard<safe::Safe<int>>(m_safeCount);
	}

	safe::LockGuard<safe::Safe<int>> get()
 	{
  		return {m_safeCount};
	}

private:
	safe::Safe<int> m_safeCount;
};

MultithreadCount safeNbrOfLinesOfCode;
{
	auto&& nbrOfLinesOfCode = safeNbrOfLinesOfCode.get();
}
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
TEST_F(AccessTest, SafeLockableRefValueRefSharedAccess) {
	SafeLockableRefValueRefType safe(lockable, value);
	
	SafeLockableRefValueRefSharedAccessType access(safe);
	
	EXPECT_EQ(&*access, &value);
	EXPECT_EQ(&*static_cast<const SafeLockableRefValueRefSharedAccessType&>(access), &value);
	EXPECT_EQ(access.operator->(), &value);
	EXPECT_EQ(static_cast<const SafeLockableRefValueRefSharedAccessType&>(access).operator->(), &value);
	EXPECT_EQ(&access.lock.lockable, &lockable);
	EXPECT_EQ(&static_cast<const SafeLockableRefValueRefSharedAccessType&>(access).lock.lockable, &lockable);

}
TEST_F(SafeTest, SafeLockableValueSharedAccess) {
	SafeLockableValueType safe(safe::default_construct_lockable, value);
	
	SafeLockableValueSharedAccessType access(safe);
	
	EXPECT_EQ(&*access, &safe.unsafe());
	EXPECT_EQ(&*static_cast<const SafeLockableValueSharedAccessType&>(access), &safe.unsafe());
	EXPECT_EQ(access.operator->(), &safe.unsafe());
	EXPECT_EQ(static_cast<const SafeLockableValueSharedAccessType&>(access).operator->(), &safe.unsafe());
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

	static_assert(std::is_same<SafeLockableRefValueRefType::Access<DummyLock, safe::ReadOnly>::ConstPointerType, const int*>::value, "");
	static_assert(std::is_same<SafeLockableRefValueRefType::Access<DummyLock, safe::ReadOnly>::PointerType, const int*>::value, "");
	static_assert(std::is_same<SafeLockableRefValueRefType::Access<DummyLock, safe::ReadOnly>::ConstReferenceType, const int&>::value, "");
	static_assert(std::is_same<SafeLockableRefValueRefType::Access<DummyLock, safe::ReadOnly>::ReferenceType, const int&>::value, "");

	static_assert(std::is_same<SafeLockableValueType::Access<DummyLock>::ConstPointerType, const int*>::value, "");
	static_assert(std::is_same<SafeLockableValueType::Access<DummyLock>::PointerType, int*>::value, "");
	static_assert(std::is_same<SafeLockableValueType::Access<DummyLock>::ConstReferenceType, const int&>::value, "");
	static_assert(std::is_same<SafeLockableValueType::Access<DummyLock>::ReferenceType, int&>::value, "");

	static_assert(std::is_same<SafeLockableValueType::Access<DummyLock, safe::ReadOnly>::ConstPointerType, const int*>::value, "");
	static_assert(std::is_same<SafeLockableValueType::Access<DummyLock, safe::ReadOnly>::PointerType, const int*>::value, "");
	static_assert(std::is_same<SafeLockableValueType::Access<DummyLock, safe::ReadOnly>::ConstReferenceType, const int&>::value, "");
	static_assert(std::is_same<SafeLockableValueType::Access<DummyLock, safe::ReadOnly>::ReferenceType, const int&>::value, "");
}
TEST_F(AccessTest, StdUniqueLockSharedAccessToGuardLockSharedAccess) {
	SafeLockableRefValueRefType safe(lockable, value);
	
	{
		testing::InSequence _;
		EXPECT_CALL(lockable, lock());
		EXPECT_CALL(lockable, touch());
		EXPECT_CALL(lockable, unlock());
		EXPECT_CALL(lockable, touch());
	}

	{
		safe::UniqueLock<SafeLockableRefValueRefType, safe::ReadOnly> uniqueLockAccess(safe);
		{
			lockable.touch();
			safe::LockGuard<SafeLockableRefValueRefType, safe::ReadOnly> lockGuardAccess(*uniqueLockAccess, *uniqueLockAccess.lock.release(), std::adopt_lock);
		}
		lockable.touch();
	}
}
TEST_F(AccessTest, StdUniqueLockAccessToGuardLockSharedAccess) {
	SafeLockableRefValueRefType safe(lockable, value);
	
	{
		testing::InSequence _;
		EXPECT_CALL(lockable, lock());
		EXPECT_CALL(lockable, touch());
		EXPECT_CALL(lockable, unlock());
		EXPECT_CALL(lockable, touch());
	}

	{
		safe::UniqueLock<SafeLockableRefValueRefType> uniqueLockAccess(safe);
		{
			lockable.touch();
			safe::LockGuard<SafeLockableRefValueRefType, safe::ReadOnly> lockGuardAccess(*uniqueLockAccess, *uniqueLockAccess.lock.release(), std::adopt_lock);
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
		safe::UniqueLock<SafeLockableRefValueRefType> uniqueLockAccess(safe);
		{
			lockable.touch();
			safe::LockGuard<SafeLockableRefValueRefType> lockGuardAccess(*uniqueLockAccess, *uniqueLockAccess.lock.release(), std::adopt_lock);
		}
		lockable.touch();
	}
}