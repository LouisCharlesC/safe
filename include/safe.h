/*
 * safe.h
 *
 *  Created on: Sep 21, 2018
 *      Author: lcc
 */

#ifndef SAFE_H_
#define SAFE_H_

#include <mutex>

namespace safe
{
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
	template<typename ValueType, typename LockableType = std::mutex>
	class Safe
	{
		struct default_construct_lockable {};

	public:
		/**
		 * @brief A class that gives const access to a value and protects
		 * it using a std::unique_lock.
		 */
		class ConstLock
		{
		public:
			/**
			 * @brief Constructor.
			 * 
			 * @pre lockable must be unlocked.
			 * @post lockable is locked.
			 * 
			 * @param[in] lockable The lockable object.
			 * @param[in] value The protected value.
			 */
			ConstLock(LockableType& lockable, const ValueType& value);
			/**
			 * @brief Construct a new ConstLock object applying the specified
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
			ConstLock(LockableType& lockable, const ValueType& value, LockPolicy tag);

			/**
			 * @brief Const accessor function.
			 * 
			 * @return const ValueType* The protected value.
			 */
			const ValueType* operator->() const noexcept;
			/**
			 * @brief Const accessor function.
			 * 
			 * @return const ValueType& The protected value.
			 */
			const ValueType& operator*() const noexcept;

			/// The std::unique_lock that manages the lockable object.
			std::unique_lock<LockableType> lock;
		private:
			/// The protected value.
			const ValueType& m_value;
		};
		/**
		 * @brief A class that gives access to a value and protects
		 * it using a std::unique_lock.
		 */
		class Lock
		{
		public:
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
			 * @return const ValueType* The protected value.
			 */
			const ValueType* operator->() const noexcept;
			/**
			 * @brief Accessor function.
			 * @return ValueType* The protected value.
			 */
			ValueType* operator->() noexcept;
			/**
			 * @brief Const accessor function.
			 * @return const ValueType& The protected value.
			 */
			const ValueType& operator*() const noexcept;
			/**
			 * @brief Const accessor function.
			 * @return ValueType& The protected value.
			 */
			ValueType& operator*() noexcept;

			/// The std::unique_lock that manages the lockable object.
			std::unique_lock<LockableType> lock;
    private:
			/// The protected value.
			ValueType& m_value;
		};

		/**
		 * @brief A class that gives const access to a value and protects
		 * it using a std::lock_guard.
		 * 
		 * Instances of this class cannot be copied around and transfered 
		 * from a scope to another due to the std::lock_guard member variable.
		 * This is the intended behavior although it makes certain operations
		 * harder, like returning a ConstGuard object. The two way I know you
		 * can transfer a ConstGuard object is by returning by list-initialization
		 * and passing to a function by rvalue reference.
		 */
		class ConstGuard
		{
		public:
			/**
			 * @brief Constructor.
			 * 
			 * @pre lockable must be unlocked
			 * @post lockable is locked
			 * 
			 * @param lockable The lockable object
			 * @param value The protected value
			 */
			ConstGuard(LockableType& lockable, const ValueType& value);
			/**
			 * @brief Construct a new ConstGuard object from a locked lockable.
			 * 
			 * @pre lockable must be locked
			 * 
			 * @param lockable The lockable object
			 * @param value The protected value
			 * @param tag An instance of std::adopt_lock_t
			 */
			ConstGuard(LockableType& lockable, const ValueType& value, std::adopt_lock_t tag);

			/**
			 * @brief Const accessor functions.
			 * @return const ValueType* The protected value.
			 */
			const ValueType* operator->() const noexcept;
			/**
			 * @brief Const accessor functions.
			 * @return const ValueType& The protected value.
			 */
			const ValueType& operator*() const noexcept;

		private:
			/// The std::lock_guard that manages the lockable object.
			const std::lock_guard<LockableType> m_guard;
			/// The protected value.
			const ValueType& m_value;
		};
		/**
		 * @brief A class that gives access to a value and protects
		 * it using a std::lock_guard.
		 * 
		 * Instances of this class cannot be copied around and transfered 
		 * from a scope to another due to the std::lock_guard member variable.
		 * This is the intended behavior although it makes certain operations
		 * harder, like returning a ConstGuard object.
		 */
		class Guard
		{
		public:
			/**
			 * @brief Constructor.
			 * 
			 * @pre lockable must be unlocked
			 * @post lockable is locked
			 * 
			 * @param lockable The lockable object
			 * @param value The protected value
			 */
			Guard(LockableType& lockable, ValueType& value);
			/**
			 * @brief Construct a new ConstGuard object from a locked lockable.
			 * 
			 * @pre lockable must be locked
			 * 
			 * @param lockable The lockable object
			 * @param value The protected value
			 * @param tag An instance of std::adopt_lock_t
			 */
			Guard(LockableType& lockable, ValueType& value, std::adopt_lock_t);

			/**
			 * @brief Const accessor function.
			 * @return const ValueType* The protected value.
			 */
			const ValueType* operator->() const noexcept;
			/**
			 * @brief Accessor function.
			 * @return ValueType* The protected value.
			 */
			ValueType* operator->() noexcept;
			/**
			 * @brief Const accessor function.
			 * @return const ValueType& The protected value.
			 */
			
			const ValueType& operator*() const noexcept;
			/**
			 * @brief Accessor function.
			 * @return ValueType& The protected value.
			 */
			ValueType& operator*() noexcept;

		private:
			/// The std::lock_guard that manages the lockable object.
      const std::lock_guard<LockableType> m_guard;
			/// The protected value.
			ValueType& m_value;
		};

		/**
		 * @brief Construct.
		 * 
		 * @tparam ValueArgs Perfect forwarding types to construct the value.
		 * @param lockable The lockable object.
		 * @param valueArgs Perfect forwarding arguments to construc the value.
		 */
		template<typename LockableArg, typename... ValueArgs>
		Safe(LockableArg&& lockableArg, ValueArgs&&... valueArgs);
		/**
		 * @brief Construct.
		 * 
		 * @tparam ValueArgs Perfect forwarding types to construct the value.
		 * @param tag Indicates that the lockable object should be default constructed.
		 * @param valueArgs Perfect forwarding arguments to construc the value.
		 */
		template<typename... ValueArgs>
		Safe(default_constructed_lockable tag, ValueArgs&&... valueArgs);

		/**
		 * @brief %Safe access to the protected value through a ConstGuard object.
		 * 
		 * @return ConstGuard
		 */
		ConstGuard constGuard() const;
		/**
		 * @brief %Safe access to the protected value through a ConstGuard object.
		 * 
		 * @return ConstGuard
		 */
		ConstGuard guard() const;
		/**
		 * @brief %Safe access to the protected value through a Guard object.
		 * 
		 * @return Guard
		 */
		Guard guard();
		/**
		 * @brief %Safe access to the protected value through a ConstLock object.
		 * 
		 * @return ConstLock
		 */
		ConstLock constLock() const;
		/**
		 * @brief %Safe access to the protected value through a ConstLock object.
		 * 
		 * @return ConstLock
		 */
		ConstLock lock() const;
		/**
		 * @brief %Safe access to the protected value through a Lock object.
		 * 
		 * @return Lock
		 */
		Lock lock();

		/**
		 * @brief Unsafe const accessor function.
		 * 
		 * @return const ValueType& The unprotected value.
		 */
		const ValueType& unsafe() const;
		/**
		 * @brief Unsafe accessor function.
		 * 
		 * @return ValueType& The unprotected value.
		 */
		ValueType& unsafe();

			/// The lockable object that protects the value.
		LockableType lockable;
	private:
			/// The value to protect.
		ValueType m_value;
	};
}  // namespace safe

#endif /* SAFE_H_ */
