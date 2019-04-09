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
	template<typename ValueType>
	class State
	{
		using SafeValue = safe::Safe<ValueType>;

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
	 * @tparam ValueType 
	 */
	template<typename ValueType>
	class State<std::shared_ptr<ValueType>>
	{
		using SafeValue = safe::Safe<std::shared_ptr<ValueType>>;
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

			; // need to release the unique_lock to create a lock_guard, sad
			// do not throw an exception between those two lines, else the mutex will remain locked, forever!
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
