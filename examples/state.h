/**
 * @file state.h
 * @author L.-C. C.
 * @brief 
 * @version 0.1
 * @date 2018-10-23
 * 
 * @copyright Copyright (c) 2018
 * 
 */

#pragma once

#include "safe/safe.h"

#include <memory>
#include <mutex>
#include <utility>

namespace mess {
	/**
	 * @brief Multi-threading utility class for variables that are meant
	 * to be modified and read by several threads.
	 * 
	 * To replace the content of a State variable, call set(). To get a
	 * copy of it, call get(). To perform arbitrary operations of a State
	 * variable, call update(). Call read() to perform arbitrary read-only
	 * operations. update() and read() yield safe::Access objects. 
	 * 
	 * @tparam ValueType Type of the State object.
	 * @tparam LockableType The type of the lockable object. Use a shared
	 * lockable if possible.
	 * @tparam DefaultReadOnlyLock The lock type to use as default for
	 * read only accesses. Use a shared lock if possible.
	 */

	template<typename ValueType, typename LockableType = std::mutex, template<typename> class DefaultReadOnlyLock = std::lock_guard>
	class State
	{
		using SafeValue = safe::Safe<ValueType, LockableType, DefaultReadOnlyLock>;

	public:
    using Handle = ValueType;

		template<typename... Args>
		State(Args&&... args):
			m_state(safe::default_construct_lockable, std::forward<Args>(args)...)
		{}

		template<typename... Args>
		void set(Args&&... args)
		{
			*m_state.writeAccess() = ValueType(std::forward<Args>(args)...);
		}

		typename SafeValue::template WriteAccess<> update()
		{
			return {m_state.unsafe(), m_state.lockable()};
		}

		typename SafeValue::template ReadAccess<> read() const
		{
			return {m_state.unsafe(), m_state.lockable()};
		}

		Handle get() const
		{
			return *m_state.readAccess();
		}

	private:
		SafeValue m_state;
	};

	/**
	 * @brief Copy-on-write optimization for State objects of std::shared_ptr!
	 * 
	 * Calls to set() replace the existing State object if possible,
	 * otherwise they allocate a new one. update() also allocates a
	 * new object only if needed. read() and get() return
	 * std::shared_ptrs, they never make copies.
	 * 
	 * @tparam ValueType The type of std::shared_ptr's pointee.
	 * @tparam LockableType The type of the lockable object. Use a shared
	 * lockable if possible.
	 * @tparam DefaultReadOnlyLock The lock type to use as default for
	 * read only accesses. Use a shared lock if possible.
	 */
	template<typename ValueType, typename LockableType, template<typename> class DefaultReadOnlyLock>
	class State<std::shared_ptr<ValueType>, LockableType, DefaultReadOnlyLock>
	{
		using SafeValue = safe::Safe<std::shared_ptr<ValueType>, LockableType, DefaultReadOnlyLock>;
	public:
    using Handle = std::shared_ptr<const ValueType>;

		template<typename... Args>
		State(Args&&... args):
			m_state(safe::default_construct_lockable, std::make_shared<ValueType>(std::forward<Args>(args)...))
		{}

		template<typename... Args>
		void set(Args&&... args)
		{
			auto&& state = m_state.writeAccess();

			// If Handles on the State do exist
			if (!state->unique())
			{
				// Construct a new shared_pointer
				*state = std::make_shared<ValueType>(std::forward<Args>(args)...);
			}
			else // no one owns a Handle on the State
			{
				// replace the contents of the shared_ptr
				**state = ValueType(std::forward<Args>(args)...);
			}
		}

		typename SafeValue::template Access<> update()
		{
			auto state = m_state.template writeAccess<std::unique_lock>();

			// If Handles on the State do exist
			if (!state->unique())
			{
				// Create a brand new copy of the state, it is now unique
				*state = std::make_shared<ValueType>(**state);
			}

			return {*state, *state.lock.release(), std::adopt_lock};
		}

		typename SafeValue::template Access<std::lock_guard, safe::AccessMode::ReadOnly> read() const
		{
			return {m_state.unsafe(), m_state.lockable()};
		}

		Handle get() const
		{
			return std::const_pointer_cast<const ValueType>(*m_state.readAccess());
		}

	private:
		SafeValue m_state;
	};
}  // namespace mess
