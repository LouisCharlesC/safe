/**
 * @file resource.h
 * @author L.-C. C.
 * @brief 
 * @version 0.1
 * @date 2018-10-11
 * 
 * @copyright Copyright (c) 2018
 * 
 */

#pragma once

#include "lockonce.h"
#include "spinmutex.h"

#include "safe/safe.h"

#include <array>
#include <cassert>
#include <mutex>
#include <thread>
#include <vector>

namespace mess {
	template<typename ValueType, std::size_t Size=0>
	class Resource: private std::array<ValueType, Size>
	{
	public:
		using Handle = typename safe::Safe<ValueType&, SpinMutex&>::template WriteAccess<LockOnce>;

		using std::array<ValueType, Size>::array;
		Resource(const ValueType& value)
		{
			this->fill(value);
		}

		Handle get()
		{
			while (true)
			{
				for (std::size_t index = 0; index != Size; ++index)
				{
					Handle resource((*this)[index], m_mutexes[index], std::try_to_lock);
					if (resource.lock.owns_lock())
					{
						return resource;
					}
				}
				std::this_thread::yield();
			}
		}

	private:
		std::array<SpinMutex, Size> m_mutexes;
	};

	template<typename ValueType>
	class Resource<ValueType, 0>: private std::vector<ValueType>
	{
	public:
		using Handle = typename safe::Safe<ValueType&, SpinMutex&>::template WriteAccess<LockOnce>;

		using std::vector<ValueType>::vector;

		Handle get()
		{
			assert(!m_mutexes.empty());

			while (true)
			{
				for (std::size_t index = 0; index != m_mutexes.size(); ++index)
				{
					Handle resource(*this[index], m_mutexes[index], std::try_to_lock);
					if (resource.lock.owns_lock())
					{
						return resource;
					}
				}
				std::this_thread::yield();
			}
		}

	private:
		std::vector<SpinMutex> m_mutexes{this->size()};
	};
}	// namespace mess
