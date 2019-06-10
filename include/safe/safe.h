/**
 * @file safe.h
 * @author L.-C. C.
 * @brief
 * @version 0.1
 * @date 2018-10-23
 *
 * @copyright Copyright (c) 2018
 *
 */

#pragma once

#include "lockable.h"

#include <memory>
#include <mutex>
#include <utility>

namespace safe {
	namespace {
		namespace impl
		{
			template<typename ValueType>
			void assign(ValueType& value, const ValueType& other)
			{
				value = other;
			}
			template<typename ValueType>
			void assign(ValueType& value, ValueType&& other)
			{
				value = std::move(other);
			}
			template<typename ValueType, typename... Args>
			void assign(ValueType& value, Args&&... args)
			{
				value = ValueType(std::forward<Args>(args)...);
			}
		}
	}
	/**
	 * @brief Multi-threading utility class for variables that are meant
	 * to be modified and read by several threads.
	 *
	 * To replace the content of a Safe variable, assign() to it. To get
	 * a copy of it, call copy(). To perform arbitrary operations of a
	 * Safe variable, call writeAccess(). Call readAccess() to perform
	 * arbitrary read-only operations.
	 *
	 * @tparam ValueType The type of the value object.
	 * @tparam MutexType The type of mutex.
	 */
	template<typename ValueType, typename MutexType = DefaultMutex, template<typename> class ReadOnlyLockType=DefaultReadOnlyLock>
	class Safe
	{
		using LockableValue = Lockable<ValueType, MutexType>;

	public:
		template<typename... Args>
		Safe(Args&&... args):
			m_value(default_construct_mutex, std::forward<Args>(args)...)
		{}

		template<typename... Args>
		void assign(Args&&... args)
		{
			impl::assign(*WriteAccess<LockableValue>(m_value), std::forward<Args>(args)...);
		}

		WriteAccess<LockableValue> writeAccess()
		{
			return {m_value};
		}

		template<template<typename> class LockType=DefaultReadOnlyLock>
		ReadAccess<LockableValue, LockType> readAccess() const
		{
			return {m_value};
		}

		template<template<typename> class LockType=DefaultReadOnlyLock>
		ValueType copy() const
		{
			return *ReadAccess<LockableValue, LockType>(m_value);
		}

	private:
		LockableValue m_value;
	};

	/**
	 * @brief Copy-on-write optimization for Safe objects of std::shared_ptr!
	 *
	 * In this specialization, calls to copy() do not necessarily cause
	 * a copy of the value object to happen. copy() returns a
	 * std::shared_ptr<const ValueType>, subsequent calls to assign() and
	 * writeAccess() might cause a copy to happen, but only if it is
	 * unsafe to directly assign to the value object (this is the
	 * copy-on-write optimization). Calls to readAccess() never make
	 * copies.
	 *
	 * @tparam ValueType The type of std::shared_ptr's pointee.
	 * @tparam MutexType The type of mutex.
	 */
	template<typename ValueType, typename MutexType, template<typename> class ReadOnlyLockType>
	class Safe<std::shared_ptr<ValueType>, MutexType, ReadOnlyLockType>
	{
		using LockableValue = Lockable<std::shared_ptr<ValueType>, MutexType>;

	public:
		Safe(std::unique_ptr<ValueType>&& other):
			m_value(default_construct_mutex, std::move(other))
		{}
		template<typename... Args>
		Safe(Args&&... args):
			m_value(default_construct_mutex, std::make_shared<ValueType>(std::forward<Args>(args)...))
		{}

		void assign(std::unique_ptr<ValueType>&& other)
		{
			*WriteAccess<LockableValue>(m_value) = std::move(other);
		}
		template<typename... Args>
		void assign(Args&&... args)
		{
			WriteAccess<LockableValue> valueAccess(m_value);

			// If the shared_ptr is not unique (it is currently shared)
			if (!valueAccess->unique())
			{
				// Construct a new shared_ptr, copying the current value
				*valueAccess = std::make_shared<ValueType>(std::forward<Args>(args)...);
				// this newly constructed shared_ptr is now unique
			}
			else // the shared_ptr is not currently shared
			{
				// replace the contents of the shared_ptr
				impl::assign(**valueAccess, std::forward<Args>(args)...);
			}
		}

		WriteAccess<LockableValue> writeAccess()
		{
			WriteAccess<LockableValue, std::unique_lock> valueAccess(m_value);

			// If the shared pointer is currently shared
			if (!valueAccess->unique())
			{
				// Construct a new shared_ptr, copying the current value
				*valueAccess = std::make_shared<ValueType>(**valueAccess);
				// this newly constructed shared_ptr is now unique
			}

			return {valueAccess};
		}

		ReadAccess<LockableValue, ReadOnlyLockType> readAccess() const
		{
			return {m_value};
		}

		std::shared_ptr<const ValueType> copy() const
		{
			// return a shared_ptr with read-only access
			return std::const_pointer_cast<const ValueType>(*ReadAccess<LockableValue, ReadOnlyLockType>(m_value));
		}

	private:
		LockableValue m_value;
	};
}  // namespace safe
