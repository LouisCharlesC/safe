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
	template<typename ValueType, typename LockableType>
	Safe<ValueType, LockableType>::SharedLock::SharedLock(LockableType& lockable, const ValueType& value):
		lock(lockable),
		m_value(value)
	{}
	template<typename ValueType, typename LockableType>
	template<typename LockPolicy>
	Safe<ValueType, LockableType>::SharedLock::SharedLock(LockableType& lockable, const ValueType& value, LockPolicy):
		lock(lockable, LockPolicy()),
		m_value(value)
	{}
	template<typename ValueType, typename LockableType>
	Safe<ValueType, LockableType>::Lock::Lock(LockableType& lockable, ValueType& value):
		lock(lockable),
		m_value(value)
	{}
	template<typename ValueType, typename LockableType>
	template<typename LockPolicy>
	Safe<ValueType, LockableType>::Lock::Lock(LockableType& lockable, ValueType& value, LockPolicy):
		lock(lockable, LockPolicy()),
		m_value(value)
	{}
	template<typename ValueType, typename LockableType>
	const ValueType* Safe<ValueType, LockableType>::SharedLock::operator->() const noexcept
	{
		return &m_value;
	}
	template<typename ValueType, typename LockableType>
	const ValueType* Safe<ValueType, LockableType>::Lock::operator->() const noexcept
	{
		return &m_value;
	}
	template<typename ValueType, typename LockableType>
	ValueType* Safe<ValueType, LockableType>::Lock::operator->() noexcept
	{
		return &m_value;
	}
	template<typename ValueType, typename LockableType>
	const ValueType& Safe<ValueType, LockableType>::SharedLock::operator*() const noexcept
	{
		return m_value;
	}
	template<typename ValueType, typename LockableType>
	const ValueType& Safe<ValueType, LockableType>::Lock::operator*() const noexcept
	{
		return m_value;
	}
	template<typename ValueType, typename LockableType>
	ValueType& Safe<ValueType, LockableType>::Lock::operator*() noexcept
	{
		return m_value;
	}

	/**
	 * Guard
	 */
	template<typename ValueType, typename LockableType>
	Safe<ValueType, LockableType>::SharedGuard::SharedGuard(LockableType& lockable, const ValueType& value):
		m_guard(lockable),
		m_value(value)
	{}
	template<typename ValueType, typename LockableType>
	Safe<ValueType, LockableType>::SharedGuard::SharedGuard(LockableType& lockable, const ValueType& value, std::adopt_lock_t):
		m_guard(lockable, std::adopt_lock_t()),
		m_value(value)
	{}
	template<typename ValueType, typename LockableType>
	Safe<ValueType, LockableType>::Guard::Guard(LockableType& lockable, ValueType& value):
		m_guard(lockable),
		m_value(value)
	{}
	template<typename ValueType, typename LockableType>
	Safe<ValueType, LockableType>::Guard::Guard(LockableType& lockable, ValueType& value, std::adopt_lock_t):
		m_guard(lockable, std::adopt_lock_t()),
		m_value(value)
	{}

  template<typename ValueType, typename LockableType>
  const ValueType* Safe<ValueType, LockableType>::SharedGuard::operator->() const noexcept
  {
    return &m_value;
  }
  template<typename ValueType, typename LockableType>
  const ValueType* Safe<ValueType, LockableType>::Guard::operator->() const noexcept
  {
    return &m_value;
  }
  template<typename ValueType, typename LockableType>
  ValueType* Safe<ValueType, LockableType>::Guard::operator->() noexcept
  {
    return &m_value;
  }

  template<typename ValueType, typename LockableType>
  const ValueType& Safe<ValueType, LockableType>::SharedGuard::operator*() const noexcept
  {
    return m_value;
  }
  template<typename ValueType, typename LockableType>
  const ValueType& Safe<ValueType, LockableType>::Guard::operator*() const noexcept
  {
    return m_value;
  }
  template<typename ValueType, typename LockableType>
  ValueType& Safe<ValueType, LockableType>::Guard::operator*() noexcept
  {
    return m_value;
  }

	/**
	 * Safe
	 */
  template<typename ValueType, typename LockableType>
  template<typename LockableArg, typename... ValueArgs>
  Safe<ValueType, LockableType>::Safe(LockableArg&& lockableArg, ValueArgs&&... valueArgs):
		lockable(std::forward<LockableArg>(lockableArg)),
		m_value(std::forward<ValueArgs>(valueArgs)...)
  {}
  template<typename ValueType, typename LockableType>
  template<typename... ValueArgs>
  Safe<ValueType, LockableType>::Safe(default_construct_lockable, ValueArgs&&... valueArgs):
		lockable(),
		m_value(std::forward<ValueArgs>(valueArgs)...)
  {}

  template<typename ValueType, typename LockableType>
  typename Safe<ValueType, LockableType>::SharedGuard Safe<ValueType, LockableType>::sharedGuard() const
  {
    return {lockable, m_value};
  }
  template<typename ValueType, typename LockableType>
  typename Safe<ValueType, LockableType>::SharedGuard Safe<ValueType, LockableType>::guard() const
  {
    return {lockable, m_value};
  }
  template<typename ValueType, typename LockableType>
  typename Safe<ValueType, LockableType>::Guard Safe<ValueType, LockableType>::guard()
  {
    return {lockable, m_value};
  }
  template<typename ValueType, typename LockableType>
  typename Safe<ValueType, LockableType>::SharedLock Safe<ValueType, LockableType>::sharedLock() const
  {
    return {lockable, m_value};
  }
  template<typename ValueType, typename LockableType>
  typename Safe<ValueType, LockableType>::SharedLock Safe<ValueType, LockableType>::lock() const
  {
    return {lockable, m_value};
  }
  template<typename ValueType, typename LockableType>
  typename Safe<ValueType, LockableType>::Lock Safe<ValueType, LockableType>::lock()
  {
    return {lockable, m_value};
  }

  template<typename ValueType, typename LockableType>
  const ValueType& Safe<ValueType, LockableType>::unsafe() const
	{
  	return m_value;
	}
  template<typename ValueType, typename LockableType>
  ValueType& Safe<ValueType, LockableType>::unsafe()
	{
  	return m_value;
	}
}  // namespace safe

#endif /* SAFE_HPP_ */
