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
		 * @brief A class that gives const access to a value and protects
		 * it using a std::unique_lock.
		 */
		class SharedLock
		{
		public:
		// SharedLock(const Safe& safe);
			/**
			 * @brief Constructor.
			 * 
			 * @pre lockable must be unlocked.
			 * @post lockable is locked.
			 * 
			 * @param[in] lockable The lockable object.
			 * @param[in] value The protected value.
			 */
			SharedLock(LockableType& lockable, ConstValueReferenceType value);
			/**
			 * @brief Construct a new SharedLock object applying the specified
			 * locking policy.
			 * 
			 * @pre When LockPolicy is std::adopt_lock_t, lockable must be locked. When
			 * LockPolicy is std::defer_lock_t, lockable must be unlocked.
			 * @post When LockPolicy is std::defer_lock_t, lockable is unlocked. Otherwise
			 * lockable is locked.
			 * 
			 * @tparam LockPolicy std::adopt_lock_t, std::try_to_lock_t
			 * or std::defer_lock_t.
			 * @param[in] lockable The lockable object.
			 * @param[in] value The protected value.
			 * @param[in] tag Dictates the locking policy to apply.
			 */
			template<typename LockPolicy>
			SharedLock(LockableType& lockable, ConstValueReferenceType value, LockPolicy tag);

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
		 * @brief A class that gives access to a value and protects
		 * it using a std::unique_lock.
		 */
		class Lock
		{
		public:
			Lock(const Safe& safe);
			/**
			 * @brief Constructor.
			 * 
			 * @pre lockable must be unlocked.
			 * @post lockable is locked.
			 * 
			 * @param lockable The lockable object.
			 * @param value The protected value.
			 */
			Lock(LockableType& lockable, ValueType& value);
			/**
			 * @brief Construct a new Lock object applying the specified
			 * locking policy.
			 * 
			 * @pre When LockPolicy is std::adopt_lock_t, lockable must be locked. When
			 * LockPolicy is std::defer_lock_t, lockable must be unlocked.
			 * @post When LockPolicy is std::defer_lock_t, lockable is unlocked. Otherwise
			 * lockable is locked.
			 * 
			 * @tparam LockPolicy std::adopt_lock_t, std::try_to_lock_t
			 * or std::defer_lock_t.
			 * @param[in] lockable The lockable object.
			 * @param[in] value The protected value.
			 * @param[in] tag Dictates the locking policy to apply.
			 */
			template<typename LockPolicy>
			Lock(LockableType& lockable, ValueType& value, LockPolicy tag);

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
		 * @brief A class that gives const access to a value and protects
		 * it using a std::lock_guard.
		 * 
		 * Instances of this class cannot be copied around and transfered 
		 * from a scope to another due to the std::lock_guard member variable.
		 * This is the intended behavior although it makes certain operations
		 * harder, like returning a SharedGuard object. The two way I know you
		 * can transfer a SharedGuard object is by returning by list-initialization
		 * and passing to a function by rvalue ValueReferenceType.
		 */
		class SharedGuard
		{
		public:
			using ConstValuePointerType = const typename std::remove_reference<ValueType>::type*;
			using ConstValueReferenceType = const typename std::remove_reference<ValueType>::type&;
			
			SharedGuard(const Safe& safe) noexcept;
			SharedGuard(Safe& safe) noexcept;

			// /**
			//  * @brief Constructor.
			//  * 
			//  * @pre lockable must be unlocked
			//  * @post lockable is locked
			//  * 
			//  * @param lockable The lockable object
			//  * @param value The protected value
			//  */
			// SharedGuard(LockableType& lockable, ConstValueReferenceType value);
			// /**
			//  * @brief Construct a new SharedGuard object from a locked lockable.
			//  * 
			//  * @pre lockable must be locked
			//  * 
			//  * @param lockable The lockable object
			//  * @param value The protected value
			//  * @param tag An instance of std::adopt_lock_t
			//  */
			// SharedGuard(LockableType& lockable, ConstValueReferenceType value, std::adopt_lock_t tag);

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
			/// The std::lock_guard that manages the lockable object.
			LockableType& m_lockable;
			/// The protected value.
			ConstValueReferenceType m_value;
		};
		/**
		 * @brief A class that gives access to a value and protects
		 * it using a std::lock_guard.
		 * 
		 * Instances of this class cannot be copied around and transfered 
		 * from a scope to another due to the std::lock_guard member variable.
		 * This is the intended behavior although it makes certain operations
		 * harder, like returning a SharedGuard object.
		 */
		class Guard
		{
		public:
			using ValuePointerType = typename std::remove_reference<ValueType>::type*;
			using ConstValuePointerType = const typename std::remove_reference<ValueType>::type*;
			using ValueReferenceType = typename std::remove_reference<ValueType>::type&;
			using ConstValueReferenceType = const typename std::remove_reference<ValueType>::type&;

			Guard(const Safe& safe) noexcept;
			Guard(Safe& safe) noexcept;
			// /**
			//  * @brief Constructor.
			//  * 
			//  * @pre lockable must be unlocked
			//  * @post lockable is locked
			//  * 
			//  * @param lockable The lockable object
			//  * @param value The protected value
			//  */
			// Guard(LockableType& lockable, ValueType& value);
			// /**
			//  * @brief Construct a new SharedGuard object from a locked lockable.
			//  * 
			//  * @pre lockable must be locked
			//  * 
			//  * @param lockable The lockable object
			//  * @param value The protected value
			//  * @param tag An instance of std::adopt_lock_t
			//  */
			// Guard(LockableType& lockable, ValueType& value, std::adopt_lock_t);

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
			 * @return ValueType& The protected value.
			 */
			ValueReferenceType operator*() noexcept;

		private:
			/// The std::lock_guard that manages the lockable object.
      const std::lock_guard<typename std::remove_reference<LockableType>::type> m_guard;
			/// The protected value.
			ValueReferenceType m_value;
		};

		friend Guard;

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
