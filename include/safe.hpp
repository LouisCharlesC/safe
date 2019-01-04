/*
 * safe.hpp
 *
 *  Created on: Sep 21, 2018
 *      Author: lcc
 */

#ifndef SAFE_HPP_
#define SAFE_HPP_

#include "safe.h"

namespace safe
{
	/**
	 * Lock
	 */
	template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
	Safe<ValueType, LockableType, SharedLockType>::Lock::Lock(Safe& safe) noexcept:
		lock(safe.m_lockable),
		m_value(safe.m_value)
	{}
	template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
	typename Safe<ValueType, LockableType, SharedLockType>::ConstValuePointerType Safe<ValueType, LockableType, SharedLockType>::Lock::operator->() const noexcept
	{
		return &m_value;
	}
	template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
	typename Safe<ValueType, LockableType, SharedLockType>::ValuePointerType Safe<ValueType, LockableType, SharedLockType>::Lock::operator->() noexcept
	{
		return &m_value;
	}
	template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
	typename Safe<ValueType, LockableType, SharedLockType>::ConstValueReferenceType Safe<ValueType, LockableType, SharedLockType>::Lock::operator*() const noexcept
	{
		return m_value;
	}
	template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
	typename Safe<ValueType, LockableType, SharedLockType>::ValueReferenceType Safe<ValueType, LockableType, SharedLockType>::Lock::operator*() noexcept
	{
		return m_value;
	}

	/**
	 * SharedLock
	 */
	template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
	Safe<ValueType, LockableType, SharedLockType>::SharedLock::SharedLock(const Safe& safe) noexcept:
		lock(safe.m_lockable),
		m_value(safe.m_value)
	{}
	template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
	typename Safe<ValueType, LockableType, SharedLockType>::ConstValuePointerType Safe<ValueType, LockableType, SharedLockType>::SharedLock::operator->() const noexcept
	{
		return &m_value;
	}
	template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
	typename Safe<ValueType, LockableType, SharedLockType>::ConstValueReferenceType Safe<ValueType, LockableType, SharedLockType>::SharedLock::operator*() const noexcept
	{
		return m_value;
	}

	/**
	 * Guard
	 */
	template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
	Safe<ValueType, LockableType, SharedLockType>::Guard::Guard(Safe& safe) noexcept:
		m_guard(safe.m_lockable),
		m_value(safe.m_value)
	{}
	template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
	Safe<ValueType, LockableType, SharedLockType>::Guard::Guard(Lock& lock) noexcept:
		m_guard(*lock.lock.release(), std::adopt_lock_t()),
		m_value(*lock)
	{}
  template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
  typename Safe<ValueType, LockableType, SharedLockType>::ConstValuePointerType Safe<ValueType, LockableType, SharedLockType>::Guard::operator->() const noexcept
  {
    return &m_value;
  }
  template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
  typename Safe<ValueType, LockableType, SharedLockType>::ValuePointerType Safe<ValueType, LockableType, SharedLockType>::Guard::operator->() noexcept
  {
    return &m_value;
  }
  template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
  typename Safe<ValueType, LockableType, SharedLockType>::ConstValueReferenceType Safe<ValueType, LockableType, SharedLockType>::Guard::operator*() const noexcept
  {
    return m_value;
  }
  template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
  typename Safe<ValueType, LockableType, SharedLockType>::ValueReferenceType Safe<ValueType, LockableType, SharedLockType>::Guard::operator*() noexcept
  {
    return m_value;
  }

	/**
	 * SharedGuard
	 */
	template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
	Safe<ValueType, LockableType, SharedLockType>::SharedGuard::SharedGuard(const Safe& safe) noexcept:
		m_lockable(safe.m_lockable),
		m_value(safe.m_value)
	{
		m_lockable.lock_shared();
	}
	template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
	Safe<ValueType, LockableType, SharedLockType>::SharedGuard::SharedGuard(SharedLock& lock) noexcept:
		m_lockable(*lock.lock.release()),
		m_value(*lock)
	{}
	template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
	Safe<ValueType, LockableType, SharedLockType>::SharedGuard::~SharedGuard() noexcept
	{
		m_lockable.unlock_shared();
	}
  template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
  typename Safe<ValueType, LockableType, SharedLockType>::ConstValuePointerType Safe<ValueType, LockableType, SharedLockType>::SharedGuard::operator->() const noexcept
  {
    return &m_value;
  }
  template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
  typename Safe<ValueType, LockableType, SharedLockType>::ConstValueReferenceType Safe<ValueType, LockableType, SharedLockType>::SharedGuard::operator*() const noexcept
  {
    return m_value;
  }

	/**
	 * Safe
	 */
  template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
  template<typename... ValueArgs>
  Safe<ValueType, LockableType, SharedLockType>::Safe(default_construct_lockable, ValueArgs&&... valueArgs):
		m_lockable(),
		m_value(std::forward<ValueArgs>(valueArgs)...)
  {}
  template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
  template<typename LockableArg, typename... ValueArgs>
  Safe<ValueType, LockableType, SharedLockType>::Safe(LockableArg&& lockableArg, ValueArgs&&... valueArgs):
		m_lockable(std::forward<LockableArg>(lockableArg)),
		m_value(std::forward<ValueArgs>(valueArgs)...)
  {}
  template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
  typename Safe<ValueType, LockableType, SharedLockType>::SharedGuard Safe<ValueType, LockableType, SharedLockType>::sharedGuard() const noexcept
  {
    return {*this};
  }
  template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
  typename Safe<ValueType, LockableType, SharedLockType>::SharedGuard Safe<ValueType, LockableType, SharedLockType>::guard() const noexcept
  {
    return {*this};
  }
  template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
  typename Safe<ValueType, LockableType, SharedLockType>::Guard Safe<ValueType, LockableType, SharedLockType>::guard() noexcept
  {
    return {*this};
  }
  template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
  typename Safe<ValueType, LockableType, SharedLockType>::SharedLock Safe<ValueType, LockableType, SharedLockType>::sharedLock() const noexcept
  {
    return {*this};
  }
  template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
  typename Safe<ValueType, LockableType, SharedLockType>::SharedLock Safe<ValueType, LockableType, SharedLockType>::lock() const noexcept
  {
    return {*this};
  }
  template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
  typename Safe<ValueType, LockableType, SharedLockType>::Lock Safe<ValueType, LockableType, SharedLockType>::lock() noexcept
  {
    return {*this};
  }
  template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
  typename Safe<ValueType, LockableType, SharedLockType>::ConstValueReferenceType Safe<ValueType, LockableType, SharedLockType>::unsafe() const noexcept
	{
  	return m_value;
	}
  template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
  typename Safe<ValueType, LockableType, SharedLockType>::ValueReferenceType Safe<ValueType, LockableType, SharedLockType>::unsafe() noexcept
	{
  	return m_value;
	}
}  // namespace safe

#endif /* SAFE_HPP_ */
