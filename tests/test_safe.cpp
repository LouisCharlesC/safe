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
safe::Safe<int> value; // <-- value and mutex packaged together!
{
	auto&& safeValue = value.access(); // <-- right mutex: guaranteed!
	//  ^^ do not mind the rvalue reference, I will explain its presence later on.

	++*safeValue; // access the value using pointer semantics: * and ->
} // from here, you cannot directly access the value anymore: jolly good, since the mutex is not locked anymore!

--value.unsafe(); // <-- unprotected access: clearly expressed!
}

void readmeDefaultConstructLockableTag()
{
std::mutex aMutex;

safe::Safe<int, std::mutex> bothDefault; // lockable and value are default constructed
safe::Safe<int, std::mutex&> noDefault(aMutex, 42); // lockable and value are initialized
safe::Safe<int, std::mutex&> valueDefault(aMutex); // lockable is initialized, and value is default constructed
safe::Safe<int, std::mutex> lockableDefaultTag(safe::default_construct_lockable, 42); // lockable is default constructed, and value is initialized
safe::Safe<int, std::mutex> lockableDefaultBraces({}, 42);
}

void readmeFlexiblyConstructLock()
{
safe::Safe<int> value;
value.lockable().lock();
{
	safe::Safe<int>::Access<> safeValue(value, std::adopt_lock);
}
{
	auto&& safeValue = value.access(std::adopt_lock);
}
}

void readmeLegacy()
{
// Unsafe code below!
std::mutex mutex;
int value;
// End of usafe code!
// Safe code from here on
safe::Safe<int&, std::mutex&> safeValue(mutex, value);
// Forget about mutex and value, only use safeValue
}

void readmeConditionVariable()
{
std::condition_variable cv;
safe::Safe<int> value;
auto safeValue = value.access<std::unique_lock>();
cv.wait(safeValue.lock);
}

void readmeBasicWithoutSafe()
{
std::mutex mutex;
int value;
std::lock_guard<std::mutex> lock(mutex);
value = 42;
}

void readmeBasicWithSafe()
{
safe::Safe<int> value;
auto&& safeValue = value.access();
*safeValue = 42;
}

void readmeRValueReferenceExplained()
{
safe::Safe<int> value;
// auto safeValue = value.access(); // <-- does not compile!
auto&& safeValueLockGuard = value.access(); // need rvalue reference
auto safeValueUniqueLock = value.access<std::unique_lock>(); // no rvalue reference needed
}

void readmeOneLiner()
{
safe::Safe<std::vector<int>> vector;

// One-liner example: assign a new value to the vector
*vector.access() = std::vector<int>(1, 2);

// Another one-liner example: clear the vector
vector.access()->clear();
}

void readmeSpecifyingAccessMode()
{
safe::Safe<int> safeValue;
safe::Safe<int>::Access<std::lock_guard, safe::AccessMode::ReadOnly> value(safeValue);
auto&& sameValue = safeValue.access<safe::AccessMode::ReadOnly>();
}

template <typename ValueType>
class ClassTemplateExample
{
public:

	void accessMemberFunction()
	{
		auto&& safeValue = m_value.template access<std::lock_guard>();
	}
	void accessClass()
	{
		typename safe::Safe<ValueType>::template Access<> safeValue(m_value);
		//																						 ^^
		// Access<> is the syntax to use the default parameters of the class template.
	}
private:
	safe::Safe<ValueType> m_value;
};

void readmeSafeInTemplatedCode()
{
	ClassTemplateExample<int> example;
	example.accessMemberFunction();
	example.accessClass();
}

void readmeReturnStdLockGuard()
{
class MultithreadedCount
{
public:
	void increment()
	{
		++*m_count.access();
	}

	safe::Safe<int>::Access<> get() // Access<> defaults to std::lock_guard and ReadWrite template parameters
	{
		return {m_count};
	}

private:
	safe::Safe<int> m_count;
};

MultithreadedCount count;
count.increment(); // thread safety managed inside the function call, simple but limited
{
	// thread safety managed by the access variable, efficient and flexible!
	auto&& safeCount = count.get(); // capture the Access<std::lock_guard> object by rvalue reference
	// do anything you like with the variable!
	*safeCount = 43; 
	--*safeCount;
} // mutex automatically unlocked when scope is exited
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