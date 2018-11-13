/*
 * safe.h
 *
 *  Created on: Sep 30, 2018
 *      Author: lcc
 */

#ifndef SAFE_H_
#define SAFE_H_

#include <mutex>

namespace safe {
	template<typename ValueType, typename LockableType = std::mutex>
	class LockGuarded
	{
	public:
		LockGuarded(ValueType& ref, LockableType& lockable):
			m_ptr(&ref),
			m_lock(lockable)
		{}

		ValueType* operator->()
		{
			return m_ptr;
		}

	private:
		ValueType* m_ptr;
		std::lock_guard<LockableType> m_lock;
	};
	template<typename ValueType, typename LockableType = std::mutex>
	class UniqueLocked
	{
	public:
		UniqueLocked(ValueType& ref, LockableType& lockable):
			m_ptr(&ref),
			m_lock(lockable)
		{}

		ValueType* operator->()
		{
			return m_ptr;
		}

		operator std::unique_lock<LockableType>&()
		{
			return m_lock;
		}

		void lock()
		{
			m_lock.lock();
		}
		bool try_lock()
		{
			return m_lock.try_lock();
		}
		void unlock()
		{
			m_lock.unlock();
		}

	private:
		ValueType* m_ptr;
		std::unique_lock<LockableType> m_lock;
	};

	template<typename ValueType, typename LockableType = std::mutex>
	class Safe
	{
	public:
		Safe(ValueType& ref, LockableType& lockable):
			m_ref(ref),
			m_lockable(lockable)
		{}

		ValueType& unsafe()
		{
			return m_ref;
		}
		LockGuarded<ValueType, LockableType> lockGuard()
		{
			return {m_ref, m_lockable};
		}
		UniqueLocked<ValueType, LockableType> uniqueLock()
		{
			return UniqueLocked<ValueType, LockableType>(m_ref, m_lockable);
		}

		operator LockableType&()
		{
			return m_lockable;
		}

	private:
		ValueType& m_ref;
		LockableType& m_lockable;
	};
}  // namespace safe

#endif /* SAFE_H_ */
