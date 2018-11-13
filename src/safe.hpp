/*
 * safe.hpp
 *
 *  Created on: Sep 21, 2018
 *      Author: lcc
 */

#ifndef SAFE_HPP_
#define SAFE_HPP_

#include "safe.h"

namespace safe {
  template<typename ValueType, typename LockableType>
  SafeGuard<ValueType, LockableType>::SafeGuard(ValueType& ref, LockableType& lockable):
    m_guard(lockable),
    m_value(ref)
  {}

  template<typename ValueType, typename LockableType>
  const ValueType* SafeGuard<ValueType, LockableType>::operator->() const noexcept
  {
    return &m_value;
  }

  template<typename ValueType, typename LockableType>
  ValueType* SafeGuard<ValueType, LockableType>::operator->() noexcept
  {
    return &m_value;
  }

  template<typename ValueType, typename LockableType>
  const ValueType& SafeGuard<ValueType, LockableType>::operator*() const noexcept
  {
    return m_value;
  }

  template<typename ValueType, typename LockableType>
  ValueType& SafeGuard<ValueType, LockableType>::operator*() noexcept
  {
    return m_value;
  }

  template<typename ValueType, typename LockableType>
  SafeLock<ValueType, LockableType>::SafeLock(ValueType& ref, LockableType& lockable):
    lock(lockable),
    m_value(ref)
  {}

  template<typename ValueType, typename LockableType>
  const ValueType* SafeLock<ValueType, LockableType>::operator->() const noexcept
  {
    return &m_value;
  }

  template<typename ValueType, typename LockableType>
  ValueType* SafeLock<ValueType, LockableType>::operator->() noexcept
  {
    return &m_value;
  }

  template<typename ValueType, typename LockableType>
  const ValueType& SafeLock<ValueType, LockableType>::operator*() const noexcept
  {
    return m_value;
  }

  template<typename ValueType, typename LockableType>
  ValueType& SafeLock<ValueType, LockableType>::operator*() noexcept
  {
    return m_value;
  }

  template<typename ValueType, typename LockableType>
  template<typename... Args>
  Safe<ValueType, LockableType>::Safe(LockableType& lockable, Args&&... args):
    lockable(lockable),
    value(std::forward<Args>(args)...)
  {}

  template<typename ValueType, typename LockableType>
  SafeGuard<const ValueType, LockableType> Safe<ValueType, LockableType>::lock_guard() const
  {
    return {value, lockable};
  }
  template<typename ValueType, typename LockableType>
  SafeGuard<ValueType, LockableType> Safe<ValueType, LockableType>::lock_guard()
  {
    return {value, lockable};
  }

  template<typename ValueType, typename LockableType>
  SafeLock<const ValueType, LockableType> Safe<ValueType, LockableType>::unique_lock() const
  {
    return {value, lockable};
  }
  template<typename ValueType, typename LockableType>
  SafeLock<ValueType, LockableType> Safe<ValueType, LockableType>::unique_lock()
  {
    return {value, lockable};
  }
}  // namespace safe

#endif /* SAFE_HPP_ */
