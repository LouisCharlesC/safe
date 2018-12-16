/*
 * state.h
 *
 *  Created on: Oct. 23, 2018
 *      Author: lcc
 */

#ifndef STATE_H_
#define STATE_H_

#include <memory>
#include <mutex>
#include <utility>
#include "../include/safe.hpp"

namespace safe {
	template<typename ValueType>
	class State
	{
		using Guard = typename safe::Safe<std::shared_ptr<ValueType>>::Guard;
		using ConstGuard = typename safe::Safe<std::shared_ptr<ValueType>>::ConstGuard;
		using Lock = typename safe::Safe<std::shared_ptr<ValueType>>::Lock;
		using ConstLock = typename safe::Safe<std::shared_ptr<ValueType>>::ConstLock;

	public:
		/**
		 * @brief The result of calling get() on a State variable. The Handle is a pointer
		 * to the value of the State variable at the moment you called get(), yet, the is
		 * guaranteed not to change during the course of the Handle's lifetime.
		 */
		class Handle
		{
		public:
			Handle(const std::shared_ptr<ValueType>& state):
				m_state(state)
			{}

	    const ValueType* operator->() const noexcept
	    {
	    	return m_state.get();
	    }
	    const ValueType& operator*() const noexcept
	    {
	    	return *m_state;
	    }

		private:
			std::shared_ptr<ValueType> m_state;
		};

		template<typename... Args>
		State(Args&&... args):
			m_safeState(m_mutex, std::make_shared<ValueType>(std::forward<Args>(args)...))
		{}

		template<typename... Args>
		void set(Args&&... args)
		{
			Guard&& state = m_safeState.guard();

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

		typename safe::Safe<ValueType>::Guard update()
		{
			auto state = m_safeState.unique_lock();

			// If Handles on the State do exist
			if (!state->unique())
			{
				// Create a brand new copy of the state, it is now unique
				*state = std::make_shared<ValueType>(**state);
			}

			// return a safe guard of the State's value
			return {*state.lock.release(), **state, std::adopt_lock_t()};
		}

		Handle get() const
		{
			return {*m_safeState.guard()};
		}

	private:
		std::mutex m_mutex;
		mutable safe::Safe<std::shared_ptr<ValueType>> m_safeState;
	};
}  // namespace safe

#endif /* STATE_H_ */
