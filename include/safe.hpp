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
	Safe<ValueType, LockableType, SharedLockType>::SharedLock::SharedLock(LockableType& lockable, ConstValueReferenceType value):
		lock(lockable),
		m_value(value)
	{}
	template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
	template<typename LockPolicy>
	Safe<ValueType, LockableType, SharedLockType>::SharedLock::SharedLock(LockableType& lockable, ConstValueReferenceType value, LockPolicy):
		lock(lockable, LockPolicy()),
		m_value(value)
	{}
	template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
	Safe<ValueType, LockableType, SharedLockType>::Lock::Lock(LockableType& lockable, ValueType& value):
		lock(lockable),
		m_value(value)
	{}
	template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
	template<typename LockPolicy>
	Safe<ValueType, LockableType, SharedLockType>::Lock::Lock(LockableType& lockable, ValueType& value, LockPolicy):
		lock(lockable, LockPolicy()),
		m_value(value)
	{}
	template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
	typename Safe<ValueType, LockableType, SharedLockType>::ConstValuePointerType Safe<ValueType, LockableType, SharedLockType>::SharedLock::operator->() const noexcept
	{
		return &m_value;
	}
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
	typename Safe<ValueType, LockableType, SharedLockType>::ConstValueReferenceType Safe<ValueType, LockableType, SharedLockType>::SharedLock::operator*() const noexcept
	{
		return m_value;
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
	 * Guard
	 */
	// template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
	// Safe<ValueType, LockableType, SharedLockType>::SharedGuard::SharedGuard(LockableType& lockable, const ValueType& value):
	// 	SharedGuard(lockable, value, std::adopt_lock_t())
	// {
	// 	m_lockable.lock_shared();
	// }
	// template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
	// Safe<ValueType, LockableType, SharedLockType>::SharedGuard::SharedGuard(LockableType& lockable, const ValueType& value, std::adopt_lock_t):
	// 	m_lockable(lockable),
	// 	m_value(value)
	// {}
	template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
	Safe<ValueType, LockableType, SharedLockType>::SharedGuard::SharedGuard(const Safe& safe) noexcept:
		m_lockable(safe.m_lockable),
		m_value(safe.m_value)
	{
		m_lockable.lock_shared();
	}
	template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
	Safe<ValueType, LockableType, SharedLockType>::SharedGuard::SharedGuard(Safe& safe) noexcept:
		m_lockable(safe.m_lockable),
		m_value(safe.m_value)
	{
		m_lockable.lock_shared();
	}
	template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
	Safe<ValueType, LockableType, SharedLockType>::SharedGuard::~SharedGuard() noexcept
	{
		m_lockable.unlock_shared();
	}
	template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
	Safe<ValueType, LockableType, SharedLockType>::Guard::Guard(const Safe& safe) noexcept:
		m_guard(safe.m_lockable),
		m_value(safe.m_value)
	{}
	template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
	Safe<ValueType, LockableType, SharedLockType>::Guard::Guard(Safe& safe) noexcept:
		m_guard(safe.m_lockable),
		m_value(safe.m_value)
	{}
	// template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
	// Safe<ValueType, LockableType, SharedLockType>::Guard::Guard(Safe& safe):
	// 	m_guard(safe.m_lockable),
	// 	m_value(safe.m_value)
	// {}

  template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
  typename Safe<ValueType, LockableType, SharedLockType>::SharedGuard::ConstValuePointerType Safe<ValueType, LockableType, SharedLockType>::SharedGuard::operator->() const noexcept
  {
    return &m_value;
  }
  template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
  typename Safe<ValueType, LockableType, SharedLockType>::Guard::ConstValuePointerType Safe<ValueType, LockableType, SharedLockType>::Guard::operator->() const noexcept
  {
    return &m_value;
  }
  template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
  typename Safe<ValueType, LockableType, SharedLockType>::Guard::ValuePointerType Safe<ValueType, LockableType, SharedLockType>::Guard::operator->() noexcept
  {
    return &m_value;
  }

  template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
  typename Safe<ValueType, LockableType, SharedLockType>::SharedGuard::ConstValueReferenceType Safe<ValueType, LockableType, SharedLockType>::SharedGuard::operator*() const noexcept
  {
    return m_value;
  }
  template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
  typename Safe<ValueType, LockableType, SharedLockType>::Guard::ConstValueReferenceType Safe<ValueType, LockableType, SharedLockType>::Guard::operator*() const noexcept
  {
    return m_value;
  }
  template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
  typename Safe<ValueType, LockableType, SharedLockType>::Guard::ValueReferenceType Safe<ValueType, LockableType, SharedLockType>::Guard::operator*() noexcept
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
    return {m_lockable, m_value};
  }
  template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
  typename Safe<ValueType, LockableType, SharedLockType>::SharedLock Safe<ValueType, LockableType, SharedLockType>::lock() const noexcept
  {
    return {m_lockable, m_value};
  }
  template<typename ValueType, typename LockableType, template<typename> typename SharedLockType>
  typename Safe<ValueType, LockableType, SharedLockType>::Lock Safe<ValueType, LockableType, SharedLockType>::lock() noexcept
  {
    return {m_lockable, m_value};
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
