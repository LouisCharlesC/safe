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

#include <safe/safe.h>

#include <doctest/doctest.h>

#include <algorithm>
#include <mutex>

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
		m_copyConstructCount(++other.m_copyConstructCount),
		m_moveConstructCount(other.m_moveConstructCount),
		m_copyAssignCount(other.m_copyAssignCount),
		m_moveAssignCount(other.m_moveAssignCount),
		m_movedFrom(other.m_movedFrom)
	{}
	CountingValue(CountingValue&& other):
		m_copyConstructCount(other.m_copyConstructCount),
		m_moveConstructCount(++other.m_moveConstructCount),
		m_copyAssignCount(other.m_copyAssignCount),
		m_moveAssignCount(other.m_moveAssignCount),
		m_movedFrom(other.m_movedFrom)
	{
		other.m_movedFrom = true;
	}
	CountingValue& operator=(const CountingValue& other)
	{
		m_copyConstructCount = other.m_copyConstructCount;
		m_moveConstructCount = other.m_moveConstructCount;
		m_copyAssignCount = 1 + std::max(m_copyAssignCount, other.m_copyAssignCount);
		other.m_copyAssignCount = m_copyAssignCount;
		m_moveAssignCount = other.m_moveAssignCount;
		m_movedFrom = other.m_movedFrom;
		return *this;
	}
	CountingValue& operator=(CountingValue&& other)
	{
		m_copyConstructCount = other.m_copyConstructCount;
		m_moveConstructCount = other.m_moveConstructCount;
		m_copyAssignCount = other.m_copyAssignCount;
		m_moveAssignCount = 1 + std::max(m_moveAssignCount, other.m_moveAssignCount);
		other.m_moveAssignCount = m_moveAssignCount;
		m_movedFrom = other.m_movedFrom;
		other.m_movedFrom = true;
		return *this;
	}
	
	bool checkCounts(
		unsigned int copyConstructCount,
		unsigned int moveConstructCount,
		unsigned int copyAssignCount,
		unsigned int moveAssignCount,
		bool movedFrom) const
	{
		return
			m_copyConstructCount==copyConstructCount &&
			m_moveConstructCount==moveConstructCount &&
			m_copyAssignCount==copyAssignCount &&
			m_moveAssignCount==moveAssignCount &&
			m_movedFrom==movedFrom;
	}

private:
	mutable unsigned int m_copyConstructCount = 0;
	unsigned int m_moveConstructCount = 0;
	mutable unsigned int m_copyAssignCount = 0;
	unsigned int m_moveAssignCount = 0;
	bool m_movedFrom = false;
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
		unsigned int copyAssignCount,
		unsigned int moveAssignCount,
		bool movedFrom) const
	{
		return m_value.checkCounts(copyConstructed, moveConstructed, copyAssignCount, moveAssignCount, movedFrom);
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
	CHECK_FALSE(lock.isLocked());
	CHECK_FALSE(lock.isFaulted());
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
	CHECK(checkerCopy.checkValueCounts(1, 0, 0, 0, false));
	// Check the lock was locked when checkerCopy was constructed
	CHECK(checkerCopy.isOk());
	// Check that checker was not moved from
	CHECK(checker.checkValueCounts(1, 0, 0, 0, false));
	// Check checker and checkerCopy are different instances
	CHECK_NE(&checker, &checkerCopy);

	// const auto& checkerMove = safeChecker.move();

	// // Check that the right number of calls were made to the lock
	// CHECK(lock.checkCounts(2,0,2));
	// // Check that the lock is unlocked and not faulted
	// CHECK_FALSE(lock.isLocked());
	// CHECK_FALSE(lock.isFaulted());
	// // Check that checkerMove was copy constructed
	// CHECK(checkerMove.checkValueCounts(1, 1, 0, 0, false));
	// // Check the lock was locked when checkerMove was constructed
	// CHECK(checkerMove.isOk());
	// // Check that checker was not moved from
	// CHECK(checker.checkValueCounts(1, 1, 0, 0, true));
	// // Check checker and checkerMove are different instances
	// CHECK_NE(&checker, &checkerMove);
}

TEST_CASE("Assign member functions lock the mutex before assigning (and unlocks afterwards!)")
{
	CountingLock lock;
	Checker checker(lock);
	safe::Safe<Checker&, CountingLock&> safeChecker(lock, checker);
	Checker otherChecker(lock);

	// TODO: use a fixture (to make initial checks...)!

	safeChecker.assign<std::unique_lock>(otherChecker, std::try_to_lock);

	// Check that the right number of calls were made to the lock
	CHECK(lock.checkCounts(0,1,1));
	// Check that the lock is unlocked and not faulted
	CHECK_FALSE(lock.isLocked());
	CHECK_FALSE(lock.isFaulted());
	// Check that checker was copy assigned once
	CHECK(checker.checkValueCounts(0, 0, 1, 0, false));
	// Check the lock was locked when checker was assigned
	CHECK(checker.isOk());
	// Check that otherChecker was not moved from
	CHECK(otherChecker.checkValueCounts(0, 0, 1, 0, false));

	// Do the same thing with a move assign
	safeChecker.assign(std::move(otherChecker));

	// Check that the right number of calls were made to the lock
	CHECK(lock.checkCounts(1,1,2));
	// Check that the lock is unlocked and not faulted
	CHECK_FALSE(lock.isLocked());
	CHECK_FALSE(lock.isFaulted());
	// Check that checker was copy and move assigned once
	CHECK(checker.checkValueCounts(0, 0, 1, 1, false));
	// Check the lock was locked when checker was assigned
	CHECK(checker.isOk());
	// Check that otherChecker was moved from
	CHECK(otherChecker.checkValueCounts(0, 0, 1, 1, true));
}