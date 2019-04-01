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
int frontEndValue; // <-- do I need to lock a mutex to safely access this variable ?

{
	std::lock_guard<std::mutex> lock(frontEndMutex); // <-- is this the right mutex ?
	++frontEndValue;
}

--frontEndValue; // <-- unprotected access, is this intended ?
}

void readmeWithSafeExample()
{
std::mutex backEndMutex;
safe::Safe<int> frontEndValue; // <-- value and mutex packaged together!

{
	auto&& safeFrontEndValue = frontEndValue.writeAccess(); // <-- right mutex: guaranteed!
	//  ^^ do not mind the rvalue reference, I will explain its presence later on.

	++*safeFrontEndValue; // access the value using pointer semantics: * and ->
} // from here, you cannot directly access the value anymore: jolly good, since the mutex is not locked anymore!

--frontEndValue.unsafe(); // <-- unprotected access: clearly expressed!
}

void readmeBasicUsageWithoutSafe()
{
std::mutex mutex;
int value;
std::lock_guard<std::mutex> lock(mutex);
value = 42;
}

void readmeBasicUsageWithSafe()
{
safe::Safe<int> value;
auto&& safeValue = value.writeAccess();
//  ^^ argh, this rvalue reference again! Here is the explanation:
const auto& constSafeValueLockGuard = value.writeAccess(); // compiles but makes the object const!
auto&& safeValueLockGuard = value.writeAccess(); // rvalue reference makes it work
auto safeValueUniqueLock = value.writeAccess<std::unique_lock>(); // no rvalue reference needed
*safeValue = 42;
}

void readmeRValueReferenceExplained()
{
safe::Safe<int> value;
// auto safeValue = value.writeAccess(); // <-- does not compile!
const auto& constSafeValueLockGuard = value.writeAccess(); // compiles but makes the object const!
auto&& safeValueLockGuard = value.writeAccess(); // rvalue reference makes it work
auto safeValueUniqueLock = value.writeAccess<std::unique_lock>(); // no rvalue reference needed
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
safe::Safe<int> value; // given a safe object
value.lockable().lock(); // with the mutex already locked...
// Because the mutex is already locked, you need to pass the std::adopt_lock tag to std::lock_guard when you construct your Access object.

// Fortunately, all arguments passed to the Safe::writeAccess() function are forwarded to the lock constructor.
auto&& safeValue = value.writeAccess(std::adopt_lock);
}

void readmeLegacy()
{
std::mutex mutex;
int unsafeValue;

// Wrap the existing variables
safe::Safe<int&, std::mutex&> value(mutex, unsafeValue);
// do not use mutex and unsafeValue directly from here on!
}

void readmeConditionVariable()
{
std::condition_variable cv;
safe::Safe<int> value;
auto safeValue = value.writeAccess<std::unique_lock>();
//  ^ no rvalue reference here because we use a std::unique_lock
cv.wait(safeValue.lock);
}

void readmeOneLiner()
{
safe::Safe<std::vector<int>> vector;
// One-liner to assign a new value to the vector
*vector.writeAccess() = std::vector<int>(1, 2);
// One-liner to clear the vector
vector.writeAccess()->clear();
}

template <typename ValueType>
class Example
{
public:
	void exampleAccessType()
	{
		typename safe::Safe<ValueType>::template Access<std::lock_guard> safeValue(m_value);
		                             // ^^^^^^^^^ <-- weird syntax
	}
	void exampleWriteAccessMemberFunction()
	{
		auto&& safeValue = m_value.template writeAccess<std::lock_guard>();
		                        // ^^^^^^^^^ <-- weird syntax
	}

private:
	safe::Safe<ValueType> m_value;
};

void readmeSafeInTemplatedCode()
{
	Example<int> example;
	example.exampleAccessType();
	example.exampleWriteAccessMemberFunction();
}

void readmeReturnStdLockGuard()
{
class MultithreadedCount
{
public:
	void increment()
	{
		++*m_count.writeAccess();
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
	
	SafeLockableRefValueRefReadWriteAccessType access = safe.writeAccess<DummyLock>();
	
	EXPECT_EQ(&*access, &value);
	EXPECT_EQ(&*static_cast<const SafeLockableRefValueRefReadWriteAccessType&>(access), &value);
	EXPECT_EQ(access.operator->(), &value);
	EXPECT_EQ(static_cast<const SafeLockableRefValueRefReadWriteAccessType&>(access).operator->(), &value);
	EXPECT_EQ(&access.lock.lockable, &lockable);
	EXPECT_EQ(&static_cast<const SafeLockableRefValueRefReadWriteAccessType&>(access).lock.lockable, &lockable);
}
TEST_F(SafeTest, SafeLockableValueAccess) {
	SafeLockableValueType safe(safe::default_construct_lockable, value);
	
	SafeLockableValueReadWriteAccessType access = safe.writeAccess<DummyLock>();
	
	EXPECT_EQ(&*access, &safe.unsafe());
	EXPECT_EQ(&*static_cast<const SafeLockableValueReadWriteAccessType&>(access), &safe.unsafe());
	EXPECT_EQ(access.operator->(), &safe.unsafe());
	EXPECT_EQ(static_cast<const SafeLockableValueReadWriteAccessType&>(access).operator->(), &safe.unsafe());
}
TEST_F(SafeTest, ConstSafeLockableRefValueRefReadOnlyAccess) {
	const SafeLockableRefValueRefType safe(lockable, value);
	
	SafeLockableRefValueRefReadOnlyAccessType access = safe.readAccess<DummyLock>();
	
	EXPECT_EQ(&*access, &value);
	EXPECT_EQ(&*static_cast<const SafeLockableRefValueRefReadOnlyAccessType&>(access), &value);
	EXPECT_EQ(access.operator->(), &value);
	EXPECT_EQ(static_cast<const SafeLockableRefValueRefReadOnlyAccessType&>(access).operator->(), &value);
	EXPECT_EQ(&access.lock.lockable, &lockable);
	EXPECT_EQ(&static_cast<const SafeLockableRefValueRefReadOnlyAccessType&>(access).lock.lockable, &lockable);
}
TEST_F(SafeTest, ConstSafeLockableValueReadOnlyAccess) {
	const SafeLockableValueType safe(safe::default_construct_lockable, value);
	
	SafeLockableValueReadOnlyAccessType access = safe.readAccess<DummyLock>();
	
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