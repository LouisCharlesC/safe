/*
 * safe.h
 *
 *  Created on: Sep 21, 2018
 *      Author: lcc
 */

#ifndef SAFE_H_
#define SAFE_H_

#include <mutex>
#include <type_traits>

namespace safe
{
	struct default_construct_lockable {};

	template<typename LockableType>
	class NonShared
	{
	private:
		LockableType m_lockable;
	public:
		void lock() {m_lockable.lock();}
		bool try_lock() {return m_lockable.try_lock();}
		void unlock() {m_lockable.unlock();}

		void lock_shared() {m_lockable.lock();}
		bool try_lock_shared() {return m_lockable.try_lock();}
		void unlock_shared() {m_lockable.unlock();}
	};
	
	/**
	 * @brief A class that wraps a value and a lockable
	 * object to protect the value.
	 * 
	 * The value is hidden by the Safe class, although it can be
	 * accessed using the unsafe() function. The safe way to access
	 * the value is through the Lock and Guard classes.
	 * 
	 * @tparam ValueType The type of the shared value to protects.
	 * @tparam LockableType The type of the lockable object.
	 */
	template<typename ValueType, typename LockableType = NonShared<std::mutex>, template<typename> typename SharedLockType=std::unique_lock>
	class Safe
	{
	public:
		using ValuePointerType = typename std::remove_reference<ValueType>::type*;
		using ConstValuePointerType = const typename std::remove_reference<ValueType>::type*;
		using ValueReferenceType = typename std::remove_reference<ValueType>::type&;
		using ConstValueReferenceType = const typename std::remove_reference<ValueType>::type&;

		/**
		 * @brief A class that locks a Safe object with unique lock behavior and
		 * gives access to the Safe object's value.
		 */
		class Lock
		{
		public:
			/**
			 * @brief Locks safe with unique lock behavior and gives
			 * access to its value.
			 * 
			 * @param[in] safe The Safe object to lock.
			 */
			Lock(Safe& safe) noexcept;

			/**
			 * @brief Const accessor function.
			 * @return ConstValuePointerType The protected value.
			 */
			ConstValuePointerType operator->() const noexcept;

			/**
			 * @brief Accessor function.
			 * @return ValuePointerType The protected value.
			 */
			ValuePointerType operator->() noexcept;

			/**
			 * @brief Const accessor function.
			 * @return ConstValueReferenceType The protected value.
			 */
			ConstValueReferenceType operator*() const noexcept;

			/**
			 * @brief Const accessor function.
			 * @return ValueReferenceType The protected value.
			 */
			ValueReferenceType operator*() noexcept;

			/// The std::unique_lock that manages the lockable object.
			std::unique_lock<typename std::remove_reference<LockableType>::type> lock;
    private:
			/// The protected value.
			ValueReferenceType m_value;
		};

		/**
		 * @brief A class that locks a Safe object with shared unique lock behavior and
		 * gives access to the Safe object's value.
		 */
		class SharedLock
		{
		public:
			/**
			 * @brief Locks safe with shared unique lock behavior and gives
			 * access to its value.
			 * 
			 * @param[in] safe The Safe object to lock.
			 */
			SharedLock(const Safe& safe) noexcept;

			/**
			 * @brief Const accessor function.
			 * 
			 * @return ConstValuePointerType The protected value.
			 */
			ConstValuePointerType operator->() const noexcept;

			/**
			 * @brief Const accessor function.
			 * 
			 * @return ConstValueReferenceType The protected value.
			 */
			ConstValueReferenceType operator*() const noexcept;

			/// The std::unique_lock that manages the lockable object.
			SharedLockType<typename std::remove_reference<LockableType>::type> lock;
		private:
			/// The protected value.
			ConstValueReferenceType m_value;
		};

		/**
		 * @brief A class that locks a Safe object with lock guard behavior and
		 * gives access to the Safe object's value.
		 */
		class Guard
		{
		public:
			/**
			 * @brief Locks safe with lock guard behavior and gives
			 * access to its value.
			 * 
			 * @param[in] safe The Safe object to lock.
			 */
			Guard(Safe& safe) noexcept;
			/**
			 * @brief Adopts the lock owned by lock.
			 * 
			 * [pre] lock must be locked.
			 * 
			 * @param[in] lock The Lock object whose lock will be adopted.
			 */
			Guard(Lock& lock) noexcept;

			Guard(const Guard&) = delete;
			Guard(Guard&&) = delete;
			Guard& operator=(const Guard&) = delete;
			Guard& operator=(Guard&&) = delete;

			/**
			 * @brief Const accessor function.
			 * @return ConstValuePointerType The protected value.
			 */
			ConstValuePointerType operator->() const noexcept;

			/**
			 * @brief Accessor function.
			 * @return ValuePointerType The protected value.
			 */
			ValuePointerType operator->() noexcept;

			/**
			 * @brief Const accessor function.
			 * @return ConstValueReferenceType The protected value.
			 */
			ConstValueReferenceType operator*() const noexcept;

			/**
			 * @brief Accessor function.
			 * @return ValueReferenceType The protected value.
			 */
			ValueReferenceType operator*() noexcept;

		private:
			/// The std::lock_guard that manages the lockable object.
      const std::lock_guard<typename std::remove_reference<LockableType>::type> m_guard;
			/// The protected value.
			ValueReferenceType m_value;
		};

		/**
		 * @brief A class that locks a Safe object with shared lock guard behavior and
		 * gives access to the Safe object's value.
		 */
		class SharedGuard
		{
		public:
			/**
			 * @brief Locks safe with shared lock guard behavior and gives
			 * access to its value.
			 * 
			 * @param[in] safe The Safe object to lock.
			 */
			SharedGuard(const Safe& safe) noexcept;
			/**
			 * @brief Adopts the lock owned by lock.
			 * 
			 * [pre] lock must be locked.
			 * 
			 * @param[in] lock The SharedLock object whose lock will be adopted.
			 */
			SharedGuard(SharedLock& lock) noexcept;

			SharedGuard(const SharedGuard&) = delete;
			SharedGuard(SharedGuard&&) = delete;
			SharedGuard& operator=(const SharedGuard&) = delete;
			SharedGuard& operator=(SharedGuard&&) = delete;

			~SharedGuard() noexcept;

			/**
			 * @brief Const accessor functions.
			 * @return ConstValuePointerType The protected value.
			 */
			ConstValuePointerType operator->() const noexcept;

			/**
			 * @brief Const accessor functions.
			 * @return ConstValueReferenceType The protected value.
			 */
			ConstValueReferenceType operator*() const noexcept;

		private:
			/// The lockable object that protects m_value.
			LockableType& m_lockable;
			/// The protected value.
			ConstValueReferenceType m_value;
		};

		/**
		 * @brief Construct a new Safe object
		 */
		Safe() = default;
		/**
		 * @brief Construct a new Safe object with default construction of the lockable
		 * object and perfect forwarding of the arguments of the value's constructor.
		 * 
		 * @tparam ValueArgs Perfect forwarding types to construct the value.
		 * @param tag Indicates that the lockable object should be default constructed.
		 * @param valueArgs Perfect forwarding arguments to construct the value.
		 */
		template<typename... ValueArgs>
		Safe(default_construct_lockable tag, ValueArgs&&... valueArgs);
		/**
		 * @brief Construct a new Safe object, perfect forwarding one argument to the
		 * lockable's constructor and perfect forwarding the other arguments
		 * of the value's constructor.
		 * 
		 * @tparam LockableArg Perfect forwarding type to construct the lockable.
		 * @tparam ValueArgs Perfect forwarding types to construct the value.
		 * @param lockableArg Perfect forwarding argument to construct the lockable.
		 * @param valueArgs Perfect forwarding arguments to construct the value.
		 */
		template<typename LockableArg, typename... ValueArgs>
		Safe(LockableArg&& lockableArg, ValueArgs&&... valueArgs);

		/**
		 * @brief %Safe access to the protected value through a SharedGuard object.
		 * 
		 * @return SharedGuard
		 */
		SharedGuard sharedGuard() const noexcept;
		/**
		 * @brief %Safe access to the protected value through a SharedGuard object.
		 * 
		 * @return SharedGuard
		 */
		SharedGuard guard() const noexcept;
		/**
		 * @brief %Safe access to the protected value through a Guard object.
		 * 
		 * @return Guard
		 */
		Guard guard() noexcept;
		/**
		 * @brief %Safe access to the protected value through a SharedLock object.
		 * 
		 * @return SharedLock
		 */
		SharedLock sharedLock() const noexcept;
		/**
		 * @brief %Safe access to the protected value through a SharedLock object.
		 * 
		 * @return SharedLock
		 */
		SharedLock lock() const noexcept;
		/**
		 * @brief %Safe access to the protected value through a Lock object.
		 * 
		 * @return Lock
		 */
		Lock lock() noexcept;

		/**
		 * @brief Unsafe const accessor function.
		 * 
		 * @return ConstValueReferenceType The unprotected value.
		 */
		ConstValueReferenceType unsafe() const noexcept;
		/**
		 * @brief Unsafe accessor function.
		 * 
		 * @return ValueReferenceType The unprotected value.
		 */
		ValueReferenceType unsafe() noexcept;

	private:
		/// The lockable object that protects the value.
		LockableType m_lockable;
		/// The value to protect.
		ValueType m_value;
	};
}  // namespace safe

#endif /* SAFE_H_ */
