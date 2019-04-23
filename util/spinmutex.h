/**
 * @file spinmutex.h
 * @author L.-C. C.
 * @brief 
 * @version 0.1
 * @date 2019-01-14
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#pragma once

#include <atomic>
#include <thread>

namespace safe
{
	struct lock_on_construction_t {};
	static constexpr lock_on_construction_t lock_on_construction;

	class SpinMutex
	{
	public:
		SpinMutex() = default;

		SpinMutex(lock_on_construction_t):
			m_flag(true)
		{}

		void lock()
		{
			while (m_flag.test_and_set())
			{
				std::this_thread::yield();
			}
		}
		bool try_lock()
		{
			return !m_flag.test_and_set();
		}
		void unlock()
		{
			m_flag.clear();
		}
	private:
		std::atomic_flag m_flag = ATOMIC_FLAG_INIT;
	};
} // namespace safe