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
#include "pushlist.h"
#include "spinmutex.h"

#include "safe/lockable.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>

namespace safe {
	static constexpr std::size_t AtConstruction = 0;
	static constexpr std::size_t Dynamic = -1;

	/**
	 * @brief Multi-threading utility class that allows you to share and
	 * reuse expensive to construct objects.
	 * 
	 * Upon construction, a number Size of instances of ValueType objects
	 * are created. When, you call get(), you scan these instances to
	 * find the first available one and you acquire a handle for it; this
	 * instance is not available anymore until the handle gets destroyed.
	 * 
	 * @tparam ValueType Type of the resource.
	 * @tparam Size=0 Number of resource instances needed. Typically
	 * equal to the number of threads that need to access this resource.
	 */
	template<typename ValueType, std::size_t Size=AtConstruction>
	class Resource 
	{
	public:
		using Handle = WriteAccess<Lockable<ValueType&, SpinMutex&>, LockOnce>;

		Resource() = default;
		Resource(const ValueType& value)
		{
			m_values.fill(value);
		}

    template<typename Rep=std::chrono::nanoseconds::rep, typename Period=std::chrono::nanoseconds::period>
		Handle get(std::chrono::duration<Rep, Period> sleepDuration = std::chrono::nanoseconds(0))
		{
			while (true)
			{
				for (std::size_t index = 0; index != Size; ++index)
				{
					Handle resource(m_values[index], m_mutexes[index], std::try_to_lock);
					if (resource.lock.owns_lock())
					{
						return resource;
					}
				}
				std::this_thread::sleep_for(sleepDuration);
			}
		}

	private:
		const std::array<SpinMutex, Size> m_mutexes;
		const std::array<ValueType, Size> m_values;
	};

	/**
	 * @brief Specialization of Resource for size specified at construction.
	 * 
	 * @tparam ValueType Type of the resource
	 */
	template<typename ValueType>
	class Resource<ValueType, AtConstruction>
	{
	public:
		using Handle = WriteAccess<Lockable<ValueType&, SpinMutex&>, LockOnce>;

		Resource(std::size_t size):
			m_size(size),
			m_mutexes(new SpinMutex[m_size]),
			m_values(new ValueType[m_size])
		{}

		Resource(std::size_t size, ValueType value):
			Resource(size)
		{
			std::fill_n(*m_values, m_size, value);
		}

    template<typename Rep=std::chrono::nanoseconds::rep, typename Period=std::chrono::nanoseconds::period>
		Handle get(std::chrono::duration<Rep, Period> sleepDuration = std::chrono::nanoseconds(0))
		{
			while (true)
			{
				for (std::size_t index = 0; index != m_size; ++index)
				{
					Handle resource(m_values[index], m_mutexes[index], std::try_to_lock);
					if (resource.lock.owns_lock())
					{
						return resource;
					}
				}
				std::this_thread::sleep_for(sleepDuration);
			}
		}

	private:
		const std::size_t m_size;
		const std::unique_ptr<SpinMutex[]> m_mutexes;
		const std::unique_ptr<ValueType[]> m_values;
	};

	template<typename ValueType>
	class Resource<ValueType, Dynamic>
	{
		using SafeResource = safe::Lockable<ValueType, SpinMutex>;

	public:
		template<typename... Args>
		WriteAccess<SafeResource, LockOnce> get(Args&&... args)
		{
			auto entry = std::find_if(m_resources.begin(), m_resources.end(), [](SafeResource& resource){return resource.lockable().try_lock();});
			if (entry == m_resources.endSentry())
			{
				entry = m_resources.push(lock_on_construction, std::forward<Args>(args)...);
			}
			return {entry, std::adopt_lock};
		}

	private:
		PushList<SafeResource> m_resources;
	};
}	// namespace safe
