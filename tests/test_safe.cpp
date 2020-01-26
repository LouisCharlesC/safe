/**
 * @file test_lockable.cpp
 * @author L.-C. C.
 * @brief 
 * @version 0.1
 * @date 2018-11-24
 * 
 * @copyright Copyright (c) 2018
 * 
 */

#include "safe/safe.h"

#include "doctest/doctest.h"

#include <mutex>
// #include <vector>


class CountingLock
{
public:
	void lock()
	{
		++m_lockCount;
		m_isFaulted |= m_isLocked; // if faulted, stay faulted
		m_isLocked = true;
	}
	bool try_lock()
	{
		++m_tryCount;
		const bool ret = !m_isLocked;
		m_isLocked = true;
		return ret;
	}
	void unlock()
	{
		++m_unlockCount;
		m_isFaulted |= !m_isLocked; // if faulted, stay faulted
		m_isLocked = false;
	}

	bool isLocked() const
	{
		return m_isLocked;
	}
	bool isFaulted() const
	{
		return m_isFaulted;
	}

	bool checkCounts(unsigned int lockCount, unsigned int tryCount, unsigned int unlockCount) const
	{
		return m_lockCount==lockCount && m_tryCount==tryCount && m_unlockCount==unlockCount;
	}

private:
	bool m_isLocked = false;
	bool m_isFaulted = false;

	unsigned int m_lockCount = 0;
	unsigned int m_tryCount = 0;
	unsigned int m_unlockCount = 0;
};

class CountingValue
{
public:
	CountingValue() = default;
	CountingValue(const CountingValue& other):
		m_copyConstructed(true)
	{}
	CountingValue(CountingValue&& other):
		m_moveConstructed(true)
	{
		other.m_movedFrom = true;
	}
	CountingValue& operator=(const CountingValue& other)
	{
		++m_copyAssignCount;
		return *this;
	}
	CountingValue& operator=(CountingValue&& other)
	{
		++m_moveAssignCount;
		other.m_movedFrom = true;
		return *this;
	}
	
	bool checkCounts(
		bool copyConstructed,
		bool moveConstructed,
		bool movedFrom,
		unsigned int copyAssignCount,
		unsigned int moveAssignCount) const
	{
		return
			m_copyConstructed==copyConstructed &&
			m_moveConstructed==moveConstructed &&
			m_movedFrom==movedFrom &&
			m_copyAssignCount==copyAssignCount &&
			m_moveAssignCount==moveAssignCount;
	}

private:
	bool m_copyConstructed = false;
	bool m_moveConstructed = false;
	bool m_movedFrom = false;
	unsigned int m_copyAssignCount = 0;
	unsigned int m_moveAssignCount = 0;
};

class Checker
{
public:
	Checker(CountingLock& lock):
		m_lock(lock)
	{}
	Checker(const Checker& other):
		m_isOk(other.m_isOk && other.m_lock.isLocked() && !other.m_lock.isFaulted()),
		m_lock(other.m_lock),
		m_value(other.m_value)
	{}
	Checker(Checker&& other):
		m_isOk(other.m_isOk && other.m_lock.isLocked() && !other.m_lock.isFaulted()),
		m_lock(other.m_lock),
		m_value(std::move(other.m_value))
	{}
	Checker& operator=(const Checker& other)
	{
		m_isOk &= other.m_isOk && other.m_lock.isLocked() && !other.m_lock.isFaulted(); // if not ok, stays not ok
		m_lock = other.m_lock;
		m_value = other.m_value;
		return *this;
	}
	Checker& operator=(Checker&& other)
	{
		m_isOk &= other.m_isOk && other.m_lock.isLocked() && !other.m_lock.isFaulted(); // if not ok, stays not ok
		m_lock = other.m_lock;
		m_value = std::move(other.m_value);
		return *this;
	}

	bool isOk() const {return m_isOk;}

	bool checkValueCounts(
		bool copyConstructed,
		bool moveConstructed,
		bool movedFrom,
		unsigned int copyAssignCount,
		unsigned int moveAssignCount) const
	{
		return m_value.checkCounts(copyConstructed, moveConstructed, movedFrom, copyAssignCount, moveAssignCount);
	}

private:
	bool m_isOk = true;
	CountingLock& m_lock;
	CountingValue m_value;
};

TEST_CASE("Copy member function locks the mutex before copying (and unlocks afterwards!)")
{
	CountingLock lock;
	Checker checker(lock);
	safe::Safe<Checker&, CountingLock&> safeChecker(lock, checker);

	// Mutex, lock and checker are in the proper inital state
	CHECK(!lock.isLocked());
	CHECK(checker.isOk());
	// Safe points to the right objects
	CHECK_EQ(&safeChecker.unsafe(), &checker);
	CHECK_EQ(&safeChecker.mutex(), &lock);

	const auto& checkerCopy = safeChecker.copy();

	// Check that the right number of calls were made to the lock
	CHECK(lock.checkCounts(1,0,1));
	// Check that the lock is unlocked and not faulted
	CHECK_FALSE(lock.isLocked());
	CHECK_FALSE(lock.isFaulted());
	// Check that checkerCopy was copy constructed
	CHECK(checkerCopy.checkValueCounts(true, false, false, 0, 0));
	// Check the lock was locked when checkerCopy was constructed
	CHECK(checkerCopy.isOk());
	// Check that checker was not moved from
	CHECK(checker.checkValueCounts(false, false, false, 0, 0));
	// Check checker and checkerCopy are different instances
	CHECK_NE(&checker, &checkerCopy);
}

TEST_CASE("Copy member function locks the mutex before copying (and unlocks afterwards!)")
{
	CountingLock lock;
	Checker checker(lock);
	safe::Safe<Checker&, CountingLock&> safeChecker(lock, checker);

	// Mutex, lock and checker are in the proper inital state
	CHECK(!lock.isLocked());
	CHECK(checker.isOk());
	// Safe points to the right objects
	CHECK_EQ(&safeChecker.unsafe(), &checker);
	CHECK_EQ(&safeChecker.mutex(), &lock);

	const auto& checkerCopy = safeChecker.copy();

	// Check that the right number of calls were made to the lock
	CHECK(lock.checkCounts(1,0,1));
	// Check that the lock is unlocked and not faulted
	CHECK_FALSE(lock.isLocked());
	CHECK_FALSE(lock.isFaulted());
	// Check that checkerCopy was copy constructed
	CHECK(checkerCopy.checkValueCounts(true, false, false, 0, 0));
	// Check the lock was locked when checkerCopy was constructed
	CHECK(checkerCopy.isOk());
	// Check that checker was not moved from
	CHECK(checker.checkValueCounts(false, false, false, 0, 0));
	// Check checker and checkerCopy are different instances
	CHECK_NE(&checker, &checkerCopy);
}

TEST_CASE("Assign member functions lock the mutex before assigning (and unlocks afterwards!)")
{
	// TODO: use a fixture (to make initial checks...)!
	CountingLock lock;
	Checker checker(lock);
	safe::Safe<Checker&, CountingLock&> safeChecker(lock, checker);
	Checker otherChecker(lock);

	safeChecker.assign<std::unique_lock>(otherChecker, std::try_to_lock);

	// Check that the right number of calls were made to the lock
	CHECK(lock.checkCounts(0,1,1));
	// Check that the lock is unlocked and not faulted
	CHECK_FALSE(lock.isLocked());
	CHECK_FALSE(lock.isFaulted());
	// Check that checker was copy assigned once
	CHECK(checker.checkValueCounts(false, false, false, 1, 0));
	// Check the lock was locked when checker was assigned
	CHECK(checker.isOk());
	// Check that otherChecker was not moved from
	CHECK(otherChecker.checkValueCounts(false, false, false, 0, 0));

	// Do the same thing with a move assign
	safeChecker.assign(std::move(otherChecker));

	// Check that the right number of calls were made to the lock
	CHECK(lock.checkCounts(1,1,2));
	// Check that the lock is unlocked and not faulted
	CHECK_FALSE(lock.isLocked());
	CHECK_FALSE(lock.isFaulted());
	// Check that checker was copy and move assigned once
	CHECK(checker.checkValueCounts(false, false, false, 1, 1));
	// Check the lock was locked when checker was assigned
	CHECK(checker.isOk());
	// Check that otherChecker was moved from
	CHECK(otherChecker.checkValueCounts(false, false, true, 0, 0));
}

// TEST_CASE("Safe's first constructor argument is passed to the mutex")
// {
// 	std::mutex mutex;
// 	SUBCASE("as the only argument")
// 	{
// 		safe::Safe<int, std::mutex&> safeInt(mutex);
// 		CHECK(&safeInt.mutex() == &mutex);
// 	}
// 	SUBCASE("with another argument")
// 	{
// 		safe::Safe<int, std::mutex&> safeInt(mutex, 42);
// 		CHECK(&safeInt.mutex() == &mutex);
// 	}
// 	SUBCASE("with more arguments")
// 	{
// 		safe::Safe<std::vector<int>, std::mutex&> safeVec(mutex, 42, 42);
// 		CHECK(&safeVec.mutex() == &mutex);
// 	}
// }

// TEST_CASE("Safe's other constructor arguments are passed to the value")
// {
// 	SUBCASE("One argument")
// 	{
// 		std::vector<int> vec;
// 		safe::Safe<std::vector<int>&> safeVec({}, vec);
// 		CHECK(&vec == &safeVec.unsafe());
// 	}
// 	SUBCASE("Two arguments")
// 	{
// 		std::vector<int> vec(42, 42);
// 		safe::Safe<std::vector<int>> safeVec({}, 42, 42);
// 		CHECK(vec == safeVec.unsafe());
// 	}
// }

// TEST_CASE("Access gets Safe's mutex and value")
// {
// 	using SafeVecInt = safe::Safe<std::vector<int>>;
// 	SafeVecInt safeVec({}, 42, 42);

// 	SUBCASE("through Safe's member function")
// 	{
// 		auto vecAccess = safeVec.readAccess<std::unique_lock>();
// 		CHECK(vecAccess.lock.mutex() == &safeVec.mutex());
// 		CHECK(&*vecAccess == &safeVec.unsafe());

// 		CHECK(vecAccess.lock.owns_lock());
// 	}

// 	SUBCASE("through Safe's member alias")
// 	{
// 		SafeVecInt::ReadAccess<std::unique_lock> vecAccess(safeVec);
// 		CHECK(vecAccess.lock.mutex() == &safeVec.mutex());
// 		CHECK(&*vecAccess == &safeVec.unsafe());

// 		CHECK(vecAccess.lock.owns_lock());
// 	}

// 	SUBCASE("through safe namespace alias")
// 	{
// 		safe::ReadAccess<SafeVecInt, std::unique_lock> vecAccess(safeVec);
// 		CHECK(vecAccess.lock.mutex() == &safeVec.mutex());
// 		CHECK(&*vecAccess == &safeVec.unsafe());

// 		CHECK(vecAccess.lock.owns_lock());
// 	}
// }

// TEST_CASE("Access's constructor arguments are passed to the lock")
// {
// 	std::vector<int> vec(42, 42);
// 	std::mutex mutex;
// 	using SafeVecIntRef = safe::Safe<std::vector<int>&, std::mutex&>;
// 	SafeVecIntRef safeVec(mutex, vec);

// 	SUBCASE("through Safe's member function")
// 	{
// 		auto vecAccess = safeVec.writeAccess<std::unique_lock>(std::defer_lock);
// 		CHECK(!vecAccess.lock.owns_lock());
// 	}

// 	SUBCASE("through Safe's member alias")
// 	{
// 		SafeVecIntRef::WriteAccess<std::unique_lock> vecAccess(safeVec, std::defer_lock);
// 		CHECK(!vecAccess.lock.owns_lock());
// 	}

// 	SUBCASE("through safe namespace alias")
// 	{
// 		safe::WriteAccess<SafeVecIntRef, std::unique_lock> vecAccess(safeVec, std::defer_lock);
// 		CHECK(!vecAccess.lock.owns_lock());
// 	}
// }