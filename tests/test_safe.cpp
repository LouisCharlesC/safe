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

	using SafeLockableRefValueRefReadOnlyAccessType = SafeLockableRefValueRefType::Access<DummyLock, safe::AccessMode::ReadOnly>;
	using SafeLockableRefValueRefReadWriteAccessType = SafeLockableRefValueRefType::Access<DummyLock>;
	using SafeLockableValueReadOnlyAccessType = SafeLockableValueType::Access<DummyLock, safe::AccessMode::ReadOnly>;
	using SafeLockableValueReadWriteAccessType = SafeLockableValueType::Access<DummyLock>;

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
int value; // <-- do I need to lock a mutex to safely access this value ?
{
	std::lock_guard<std::mutex> lock(frontEndMutex); // <-- is this the right mutex ?
	++value;
}
--value; // <-- unprotected access, is this intended ?
}

void readmeWithSafeExample()
{
std::mutex frontEndMutex;
safe::Safe<int> safeValue; // <-- value and mutex packaged together!
{
	safe::Safe<int>::Access<std::lock_guard> value(safeValue); // <-- right mutex: guaranteed!
	++*value; // access the value using pointer semantics: * and ->
} // from here, you cannot directly access the value anymore: jolly good, since the mutex is not locked anymore!
--safeValue.unsafe(); // <-- unprotected access: clearly expressed!
}

void readmeDefaultConstructLockableTag()
{
std::mutex aMutex;

safe::Safe<int, std::mutex> bothDefault; // lockable and value are default constructed
safe::Safe<int, std::mutex&> noDefault(aMutex, 42); // lockable and value are initialized
safe::Safe<int, std::mutex&> valueDefault(aMutex); // lockable is initialized, and value is default constructed
safe::Safe<int, std::mutex> lockableDefault(safe::default_construct_lockable, 42); // lockable is default constructed, and value is initialized
}

void readmeBasic()
{
// with safe
safe::Safe<int> safeValue;
// ---
safe::Safe<int>::Access<std::lock_guard> value(safeValue); // with safe
// ---
*value = 42; // with safe
}

void readmeConditionVariable()
{
std::condition_variable cv;
safe::Safe<int> safeValue;
safe::Safe<int>::Access<std::unique_lock> value(safeValue);
cv.wait(value.lock);
}

void readmeOneLine()
{
safe::Safe<std::vector<int>> safeVector;
*safeVector.access() = std::vector<int>(1, 2);
safeVector.access()->clear();
}

void readmeCapturingAccess()
{
safe::Safe<int> safeValue;
// auto value = safeValue.access(); // <-- does not compile!
auto&& value = safeValue.access(); // notice the rvalue reference: auto&&
}

void readmeSpecifyingAccessMode()
{
safe::Safe<int> safeValue;
safe::Safe<int>::Access<std::lock_guard, safe::AccessMode::ReadOnly> value(safeValue);
auto&& sameValue = safeValue.access<std::lock_guard, safe::AccessMode::ReadOnly>();
}

template <typename ValueType>
class Example
{
public:
	safe::Safe<ValueType> m_safeValue;

	void exampleAccessType()
	{
		typename safe::Safe<ValueType>::template Access<std::lock_guard> value(m_safeValue);
	}
	void exampleAccessMemberFunction()
	{
		auto&& value = m_safeValue.template access<std::lock_guard>();
	}
};

void readmeSafeInTemplatedCode()
{
	Example<int> example;
	example.exampleAccessType();
	example.exampleAccessMemberFunction();
}

void readmeReturnStdLockGuard()
{
class MultithreadCount
{
public:
	void increment()
	{
		++*m_safeCount.access();
	}

	safe::Safe<int>::Access<std::lock_guard> get()
	{
		return {m_safeCount};
	}

private:
	safe::Safe<int> m_safeCount;
};

MultithreadCount count;
count.increment(); // thread safety managed inside the function call, efficient but limited
{
	auto&& countAccess = count.get(); // capture the Access<std::lock_guard> object by rvalue reference
	// do anything you like with the count variable, the mutex is locked once, and the MultithreadCount does not have foresee and implement all the operation you will perform of the variable.
	*countAccess = 43; 
	--*countAccess;
} // unlock the mutex when you are done
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
TEST_F(SafeTest, SafeLockableRefValueRefAccess) {
	SafeLockableRefValueRefType safe(lockable, value);
	
	SafeLockableRefValueRefReadWriteAccessType access = safe.access<DummyLock>();
	
	EXPECT_EQ(&*access, &value);
	EXPECT_EQ(&*static_cast<const SafeLockableRefValueRefReadWriteAccessType&>(access), &value);
	EXPECT_EQ(access.operator->(), &value);
	EXPECT_EQ(static_cast<const SafeLockableRefValueRefReadWriteAccessType&>(access).operator->(), &value);
	EXPECT_EQ(&access.lock.lockable, &lockable);
	EXPECT_EQ(&static_cast<const SafeLockableRefValueRefReadWriteAccessType&>(access).lock.lockable, &lockable);
}
TEST_F(SafeTest, SafeLockableValueAccess) {
	SafeLockableValueType safe(safe::default_construct_lockable, value);
	
	SafeLockableValueReadWriteAccessType access = safe.access<DummyLock>();
	
	EXPECT_EQ(&*access, &safe.unsafe());
	EXPECT_EQ(&*static_cast<const SafeLockableValueReadWriteAccessType&>(access), &safe.unsafe());
	EXPECT_EQ(access.operator->(), &safe.unsafe());
	EXPECT_EQ(static_cast<const SafeLockableValueReadWriteAccessType&>(access).operator->(), &safe.unsafe());
}
TEST_F(SafeTest, ConstSafeLockableRefValueRefReadOnlyAccess) {
	const SafeLockableRefValueRefType safe(lockable, value);
	
	SafeLockableRefValueRefReadOnlyAccessType access = safe.access<DummyLock>();
	
	EXPECT_EQ(&*access, &value);
	EXPECT_EQ(&*static_cast<const SafeLockableRefValueRefReadOnlyAccessType&>(access), &value);
	EXPECT_EQ(access.operator->(), &value);
	EXPECT_EQ(static_cast<const SafeLockableRefValueRefReadOnlyAccessType&>(access).operator->(), &value);
	EXPECT_EQ(&access.lock.lockable, &lockable);
	EXPECT_EQ(&static_cast<const SafeLockableRefValueRefReadOnlyAccessType&>(access).lock.lockable, &lockable);
}
TEST_F(SafeTest, ConstSafeLockableValueReadOnlyAccess) {
	const SafeLockableValueType safe(safe::default_construct_lockable, value);
	
	SafeLockableValueReadOnlyAccessType access = safe.access<DummyLock>();
	
	EXPECT_EQ(&*access, &safe.unsafe());
	EXPECT_EQ(&*static_cast<const SafeLockableValueReadOnlyAccessType&>(access), &safe.unsafe());
	EXPECT_EQ(access.operator->(), &safe.unsafe());
	EXPECT_EQ(static_cast<const SafeLockableValueReadOnlyAccessType&>(access).operator->(), &safe.unsafe());
}

TEST_F(AccessTest, ReturnTypes) {
	static_assert(std::is_same<SafeLockableRefValueRefType::Access<DummyLock>::ConstPointerType, const int*>::value, "");
	static_assert(std::is_same<SafeLockableRefValueRefType::Access<DummyLock>::PointerType, int*>::value, "");
	static_assert(std::is_same<SafeLockableRefValueRefType::Access<DummyLock>::ConstReferenceType, const int&>::value, "");
	static_assert(std::is_same<SafeLockableRefValueRefType::Access<DummyLock>::ReferenceType, int&>::value, "");

	static_assert(std::is_same<SafeLockableRefValueRefType::Access<DummyLock, safe::AccessMode::ReadOnly>::ConstPointerType, const int*>::value, "");
	static_assert(std::is_same<SafeLockableRefValueRefType::Access<DummyLock, safe::AccessMode::ReadOnly>::PointerType, const int*>::value, "");
	static_assert(std::is_same<SafeLockableRefValueRefType::Access<DummyLock, safe::AccessMode::ReadOnly>::ConstReferenceType, const int&>::value, "");
	static_assert(std::is_same<SafeLockableRefValueRefType::Access<DummyLock, safe::AccessMode::ReadOnly>::ReferenceType, const int&>::value, "");

	static_assert(std::is_same<SafeLockableValueType::Access<DummyLock>::ConstPointerType, const int*>::value, "");
	static_assert(std::is_same<SafeLockableValueType::Access<DummyLock>::PointerType, int*>::value, "");
	static_assert(std::is_same<SafeLockableValueType::Access<DummyLock>::ConstReferenceType, const int&>::value, "");
	static_assert(std::is_same<SafeLockableValueType::Access<DummyLock>::ReferenceType, int&>::value, "");

	static_assert(std::is_same<SafeLockableValueType::Access<DummyLock, safe::AccessMode::ReadOnly>::ConstPointerType, const int*>::value, "");
	static_assert(std::is_same<SafeLockableValueType::Access<DummyLock, safe::AccessMode::ReadOnly>::PointerType, const int*>::value, "");
	static_assert(std::is_same<SafeLockableValueType::Access<DummyLock, safe::AccessMode::ReadOnly>::ConstReferenceType, const int&>::value, "");
	static_assert(std::is_same<SafeLockableValueType::Access<DummyLock, safe::AccessMode::ReadOnly>::ReferenceType, const int&>::value, "");
}
// TEST_F(AccessTest, StdUniqueLockReadOnlyAccessToGuardLockReadOnlyAccess) {
// 	SafeLockableRefValueRefType safe(lockable, value);
	
// 	{
// 		testing::InSequence _;
// 		EXPECT_CALL(lockable, lock());
// 		EXPECT_CALL(lockable, touch());
// 		EXPECT_CALL(lockable, unlock());
// 		EXPECT_CALL(lockable, touch());
// 	}

// 	{
// 		SafeLockableRefValueRefType::Access<std::unique_lock, safe::ReadOnly> uniqueLockAccess(safe);
// 		{
// 			lockable.touch();
// 			SafeLockableRefValueRefType::Access<std::lock_guard, safe::ReadOnly> lockGuardAccess(*uniqueLockAccess, *uniqueLockAccess.lock.release(), std::adopt_lock);
// 		}
// 		lockable.touch();
// 	}
// }
// TEST_F(AccessTest, StdUniqueLockAccessToGuardLockReadOnlyAccess) {
// 	SafeLockableRefValueRefType safe(lockable, value);
	
// 	{
// 		testing::InSequence _;
// 		EXPECT_CALL(lockable, lock());
// 		EXPECT_CALL(lockable, touch());
// 		EXPECT_CALL(lockable, unlock());
// 		EXPECT_CALL(lockable, touch());
// 	}

// 	{
// 		SafeLockableRefValueRefType::Access<std::unique_lock> uniqueLockAccess(safe);
// 		{
// 			lockable.touch();
// 			SafeLockableRefValueRefType::Access<std::lock_guard, safe::ReadOnly> lockGuardAccess(*uniqueLockAccess, *uniqueLockAccess.lock.release(), std::adopt_lock);
// 		}
// 		lockable.touch();
// 	}
// }
// TEST_F(AccessTest, StdUniqueLockAccessToGuardLockAccess) {
// 	SafeLockableRefValueRefType safe(lockable, value);
	
// 	{
// 		testing::InSequence _;
// 		EXPECT_CALL(lockable, lock());
// 		EXPECT_CALL(lockable, touch());
// 		EXPECT_CALL(lockable, unlock());
// 		EXPECT_CALL(lockable, touch());
// 	}

// 	{
// 		SafeLockableRefValueRefType::Access<std::unique_lock> uniqueLockAccess(safe);
// 		{
// 			lockable.touch();
// 			SafeLockableRefValueRefType::Access<std::lock_guard> lockGuardAccess(*uniqueLockAccess, *uniqueLockAccess.lock.release(), std::adopt_lock);
// 		}
// 		lockable.touch();
// 	}
// }