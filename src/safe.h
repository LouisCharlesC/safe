/*
 * safe.h
 *
 *  Created on: Sep 21, 2018
 *      Author: lcc
 */

#ifndef SAFE_H_
#define SAFE_H_

#include <mutex>

namespace safe {
  template<typename ValueType, typename LockableType = std::mutex>
  class SafeGuard
  {
  public:
    SafeGuard(ValueType& ref, LockableType& lockable);

    const ValueType* operator->() const noexcept;
    ValueType* operator->() noexcept;
    const ValueType& operator*() const noexcept;
    ValueType& operator*() noexcept;

  private:
    std::unique_lock<LockableType> m_guard;
    ValueType& m_value;
  };

  template<typename ValueType, typename LockableType = std::mutex>
  class SafeLock
  {
  public:
    SafeLock(ValueType& ref, LockableType& lockable);

    const ValueType* operator->() const noexcept;
    ValueType* operator->() noexcept;
    const ValueType& operator*() const noexcept;
    ValueType& operator*() noexcept;

    std::unique_lock<LockableType> lock;
  private:
    ValueType& m_value;
  };

  template<typename ValueType, typename LockableType = std::mutex>
  struct Safe
  {
  public:
  	using Guard = SafeGuard<ValueType, LockableType>;
  	using Lock = SafeLock<ValueType, LockableType>;

    template<typename... Args>
    Safe(LockableType& lockable, Args&&... args);

    SafeGuard<const ValueType, LockableType> lock_guard() const;
    SafeGuard<ValueType, LockableType> lock_guard();
    SafeLock<const ValueType, LockableType> unique_lock() const;
    SafeLock<ValueType, LockableType> unique_lock();

  private:
    LockableType& lockable;
  public:
    ValueType value;
  };
}  // namespace safe

#endif /* SAFE_H_ */
