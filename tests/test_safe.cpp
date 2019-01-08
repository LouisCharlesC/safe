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

class DummyLockable
{
public:
	MOCK_METHOD0(touch, void());
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

// class MockValue: public std::vector<int>
// {
// public:
// 	using std::vector<int>::vector;
//   MOCK_CONST_METHOD0(touch, void());
// };

class SafeTest : public testing::Test {
public:
	using SafeLockableRefValueRefType = safe::Safe<int&, DummyLockable&>;
	using SafeLockableRefConstValueRefType = safe::Safe<const int&, DummyLockable&>;
	using SafeLockableRefValueType = safe::Safe<int, DummyLockable&>;
	using SafeLockableRefConstValueType = safe::Safe<const int, DummyLockable&>;
	using SafeLockableValueRefType = safe::Safe<int&, DummyLockable>;
	using SafeLockableValueType = safe::Safe<int, DummyLockable>;

	using SafeMutexRefValueRefSharedAccessType = SafeLockableRefValueRefType::SharedAccess<DummyLock>;
	using SafeMutexRefValueRefAccessType = SafeLockableRefValueRefType::Access<DummyLock>;
	using SafeMutexValueSharedAccessType = SafeLockableValueType::SharedAccess<DummyLock>;
	using SafeMutexValueAccessType = SafeLockableValueType::Access<DummyLock>;

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
	std::mutex wrong_mutex;
	safe::Safe<std::vector<int>> safeVec; // <-- the right mutex is in here!
	{
		safe::Safe<std::vector<int>>::Access<std::lock_guard> vec(safeVec); // <-- right mutex: guaranteed!
		vec->push_back(42);
	}
	safeVec.unsafe().pop_back(); // <-- unprotected access: clearly expressed!
}

TEST_F(SafeTest, MutexRefValueRefConstructor) {
	SafeLockableRefValueRefType safe(lockable, value);

	EXPECT_EQ(&safe.unsafe(), &value);
	EXPECT_EQ(&static_cast<const SafeLockableRefValueRefType&>(safe).unsafe(), &safe.unsafe());
}
TEST_F(SafeTest, MutexRefConstValueRefConstructor) {
	SafeLockableRefConstValueRefType safe(lockable, value);

	EXPECT_EQ(&safe.unsafe(), &value);
	EXPECT_EQ(&static_cast<const SafeLockableRefConstValueRefType&>(safe).unsafe(), &safe.unsafe());
}
TEST_F(SafeTest, MutexRefValueConstructor) {
	SafeLockableRefValueType safe(lockable, value);

	EXPECT_EQ(safe.unsafe(), value);
	EXPECT_EQ(&static_cast<const SafeLockableRefValueType&>(safe).unsafe(), &safe.unsafe());
}
TEST_F(SafeTest, MutexRefConstValueConstructor) {
	SafeLockableRefConstValueType safe(lockable, value);
#
	EXPECT_EQ(safe.unsafe(), value);
	EXPECT_EQ(&static_cast<const SafeLockableRefConstValueType&>(safe).unsafe(), &safe.unsafe());
}
TEST_F(SafeTest, MutexValueRefConstructor) {
	SafeLockableValueRefType safe(safe::default_construct_lockable, value);

	EXPECT_EQ(&safe.unsafe(), &value);
	EXPECT_EQ(&static_cast<const SafeLockableValueRefType&>(safe).unsafe(), &safe.unsafe());
}
TEST_F(SafeTest, MutexValueConstructor) {
	SafeLockableValueType safe(safe::default_construct_lockable, value);

	EXPECT_EQ(safe.unsafe(), value);
	EXPECT_EQ(&static_cast<const SafeLockableValueType&>(safe).unsafe(), &safe.unsafe());
}
TEST_F(SafeTest, SafeMutexRefValueRefFunctionSharedAccess) {
	SafeLockableRefValueRefType safe(lockable, value);
	
	auto access = safe.accessShared<DummyLock>();
	
	EXPECT_EQ(&*access, &value);
	EXPECT_EQ(&*static_cast<const SafeMutexRefValueRefSharedAccessType&>(access), &value);
	EXPECT_EQ(access.operator->(), &value);
	EXPECT_EQ(static_cast<const SafeMutexRefValueRefSharedAccessType&>(access).operator->(), &value);
	EXPECT_EQ(&access.lock.lockable, &lockable);
	EXPECT_EQ(&static_cast<const SafeMutexRefValueRefSharedAccessType&>(access).lock.lockable, &lockable);

	EXPECT_CALL(lockable, touch()).Times(2);
	access.lock.lockable.touch();
	static_cast<const SafeMutexRefValueRefSharedAccessType&>(access).lock.lockable.touch();

}
TEST_F(SafeTest, SafeMutexValueFunctionSharedAccess) {
	SafeLockableValueType safe(safe::default_construct_lockable, value);
	
	auto access = safe.accessShared<DummyLock>();
	
	EXPECT_EQ(&*access, &safe.unsafe());
	EXPECT_EQ(&*static_cast<const SafeMutexValueSharedAccessType&>(access), &safe.unsafe());
	EXPECT_EQ(access.operator->(), &safe.unsafe());
	EXPECT_EQ(static_cast<const SafeMutexValueSharedAccessType&>(access).operator->(), &safe.unsafe());
}
TEST_F(SafeTest, ConstFunctionAccess) {
	const SafeLockableRefValueRefType safe(lockable, value);
	
	auto access = safe.access<DummyLock>();
	
	EXPECT_EQ(&*access, &value);
	EXPECT_EQ(&*static_cast<const SafeMutexRefValueRefSharedAccessType&>(access), &value);
	EXPECT_EQ(access.operator->(), &value);
	EXPECT_EQ(static_cast<const SafeMutexRefValueRefSharedAccessType&>(access).operator->(), &value);
	EXPECT_EQ(&access.lock.lockable, &lockable);
	EXPECT_EQ(&static_cast<const SafeMutexRefValueRefSharedAccessType&>(access).lock.lockable, &lockable);

	EXPECT_CALL(lockable, touch()).Times(2);
	access.lock.lockable.touch();
	static_cast<const SafeMutexRefValueRefSharedAccessType&>(access).lock.lockable.touch();
}
TEST_F(SafeTest, FunctionAccess) {
	SafeLockableRefValueRefType safe(lockable, value);
	
	auto access = safe.access<DummyLock>();
	
	EXPECT_EQ(&*access, &value);
	EXPECT_EQ(&*static_cast<const SafeMutexRefValueRefAccessType&>(access), &value);
	EXPECT_EQ(access.operator->(), &value);
	EXPECT_EQ(static_cast<const SafeMutexRefValueRefAccessType&>(access).operator->(), &value);
	EXPECT_EQ(&access.lock.lockable, &lockable);
	EXPECT_EQ(&static_cast<const SafeMutexRefValueRefAccessType&>(access).lock.lockable, &lockable);

	EXPECT_CALL(lockable, touch()).Times(2);
	access.lock.lockable.touch();
	static_cast<const SafeMutexRefValueRefAccessType&>(access).lock.lockable.touch();
}

TEST_F(AccessTest, ReturnTypes) {
	static_assert(std::is_same<SafeLockableRefValueRefType::Access<DummyLock>::ConstPointerType, const int*>::value, "");
	static_assert(std::is_same<SafeLockableRefValueRefType::Access<DummyLock>::PointerType, int*>::value, "");
	static_assert(std::is_same<SafeLockableRefValueRefType::Access<DummyLock>::ConstReferenceType, const int&>::value, "");
	static_assert(std::is_same<SafeLockableRefValueRefType::Access<DummyLock>::ReferenceType, int&>::value, "");

	static_assert(std::is_same<SafeLockableRefValueRefType::SharedAccess<DummyLock>::ConstPointerType, const int*>::value, "");
	static_assert(std::is_same<SafeLockableRefValueRefType::SharedAccess<DummyLock>::PointerType, const int*>::value, "");
	static_assert(std::is_same<SafeLockableRefValueRefType::SharedAccess<DummyLock>::ConstReferenceType, const int&>::value, "");
	static_assert(std::is_same<SafeLockableRefValueRefType::SharedAccess<DummyLock>::ReferenceType, const int&>::value, "");

	static_assert(std::is_same<SafeLockableValueType::Access<DummyLock>::ConstPointerType, const int*>::value, "");
	static_assert(std::is_same<SafeLockableValueType::Access<DummyLock>::PointerType, int*>::value, "");
	static_assert(std::is_same<SafeLockableValueType::Access<DummyLock>::ConstReferenceType, const int&>::value, "");
	static_assert(std::is_same<SafeLockableValueType::Access<DummyLock>::ReferenceType, int&>::value, "");

	static_assert(std::is_same<SafeLockableValueType::SharedAccess<DummyLock>::ConstPointerType, const int*>::value, "");
	static_assert(std::is_same<SafeLockableValueType::SharedAccess<DummyLock>::PointerType, const int*>::value, "");
	static_assert(std::is_same<SafeLockableValueType::SharedAccess<DummyLock>::ConstReferenceType, const int&>::value, "");
	static_assert(std::is_same<SafeLockableValueType::SharedAccess<DummyLock>::ReferenceType, const int&>::value, "");
}