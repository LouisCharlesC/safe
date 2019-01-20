/*
 * safe.h
 *
 *  Created on: Sep 21, 2018
 *      Author: lcc
 */

#ifndef SAFE_H_
#define SAFE_H_

#include "locktraits.h"

#include <mutex>
#include <type_traits>

namespace safe
{
	enum ReadOrWrite
	{
		ReadOnly,
		ReadWrite
	};

	struct default_construct_lockable_t {};
	static constexpr default_construct_lockable_t default_construct_lockable;
	
	/**
	 * @brief Wraps a value together with the lockable object that
	 * protects it for multi-threaded access.
	 * 
	 * The Safe class does two things. First, it hides the value object
	 * and associates it with the lockable object that protects it.
	 * Second, it defines alises for the Access classes that will manage
	 * the lockable object and give access to the value.
	 * The value can also be accessed without locking the lockable object
	 * through the unsafe() functions.
	 * 
	 * @tparam ValueType The type of the value to protect.
	 * @tparam LockableType The type of the lockable object.
	 */
	template<typename ValueType, typename LockableType = std::mutex>
	class Safe
	{
	private:
		/// Type ValueType with reference removed, if present
		using RemoveRefValueType = typename std::remove_reference<ValueType>::type;
		/// Type LockableType with reference removed, if present
		using RemoveRefLockableType = typename std::remove_reference<LockableType>::type;
		
		/**
		 * @brief A helper class that defines a member variable of type
		 * MaybeReferenceLockableType. The variable is declared mutable if
		 * MaybeReferenceLockableType is not a reference, the varialbe is
		 * not mutable if MaybeReferenceLockableType is a reference.
		 * 
		 * @tparam MaybeReferenceLockableType 
		 */
		template<typename MaybeReferenceLockableType>
		struct MutableIfNotReferenceLockableType
		{
			MutableIfNotReferenceLockableType() = default;
			template<typename LockableArg>
			MutableIfNotReferenceLockableType(LockableArg&& lockableArg):
				lockable(std::forward<LockableArg>(lockableArg))
			{}
			/// Mutable Lockable object.
			mutable MaybeReferenceLockableType lockable;
		};
		template<typename ReferenceLockableType>
		struct MutableIfNotReferenceLockableType<ReferenceLockableType&>
		{
			MutableIfNotReferenceLockableType(ReferenceLockableType& lockable):
				lockable(lockable)
			{}
			/// Reference to a Lockable object.
			ReferenceLockableType& lockable;
		};

		/**
		 * @brief Manages a lockable object and gives access to the value
		 * from a Safe object.
		 * 
		 * The LockType template parameter determines the locking behavior
		 * of the AccessImpl class. LockType can for instance be 
		 * std::lock_guard or std::unique_lock. The locking behavior will
		 * be as expected. The value can be accessed through an AccessImpl
		 * object using pointer semantics * and ->.
		 * 
		 * @tparam LockType The type of the lock object that manages the
		 * lockable object.
		 * @tparam Shared Whether the access should be const.
		 */
		template<typename LockType, ReadOrWrite AccessType>
		class AccessImpl
		{
		private:
			/// ValueType with const qualifier if template parameter Shared is true.
			using ConditionallyConstValueType = typename std::conditional<AccessType==ReadOnly, const RemoveRefValueType, RemoveRefValueType>::type;

		public:
			/// Pointer-to-const ValueType
			using ConstPointerType = const ConditionallyConstValueType*;
			/// Pointer-to-const ValueType if Shared is true, pointer to
			/// ValueType otherwise.
			using PointerType = ConditionallyConstValueType*;
			/// Reference-to-const ValueType
			using ConstReferenceType = const ConditionallyConstValueType&;
			/// Reference-to-const ValueType if Shared is true, reference to
			/// ValueType otherwise.
			using ReferenceType = ConditionallyConstValueType&;

			/**
			 * @brief Construct an AccessImpl object that will manage a Safe
			 * object, locking its lockable object and exposing its value
			 * object.
			 * 
			 * @param[in] safe The Safe object to manage.
			 */
			AccessImpl(const Safe& safe) noexcept;
			/**
			 * @brief Construct an AccessImpl object that will manage a Safe
			 * object, locking its lockable object and exposing its value
			 * object.
			 * 
			 * @param[in] safe The Safe object to manage.
			 */
			AccessImpl(Safe& safe) noexcept;

			/**
			 * @brief Construct an AccessImpl object from a reference to the
			 * value to expose and perfect forwarding the other arguments to
			 * construct the LockType object.
			 * 
			 * This constructor allows one to use LockType objects with complex
			 * constructors, to change the type of the LockType or to pass
			 * locking behavior tags (like std::adopt_lock, std::try_to_lock
			 * and std::defer_lock) to the LockType object.
			 * This can be useful to change from an AccessImpl object with
			 * std::unique_lock behavior to one with std::lock_guard
			 * behavior. This is done so, with the lockable object readily
			 * locked by the uniqueLockAccess object:
			 * AccessImpl<std::lock_guard, SharedOrNot> lockguardAccess(*uniqueLockAccess, *uniqueLockAccess.lock.release(), std::adopt_lock);
			 * 
			 * @tparam LockArgs Perfect forwarding types to construct the
			 * LockType object.
			 * @param value A reference to the value to expose.
			 * @param lockArgs Perfect forwarding arguments to construct the
			 * LockType object.
			 */
			template<typename... LockArgs>
			AccessImpl(ReferenceType value, LockArgs&&... lockArgs);

			/**
			 * @brief Const accessor to the value.
			 * @return ConstPointerType Const pointer to the protected value.
			 */
			ConstPointerType operator->() const noexcept;

			/**
			 * @brief Accessor to the value.
			 * @return ValuePointerType Pointer to the protected value.
			 */
			PointerType operator->() noexcept;

			/**
			 * @brief Const accessor to the value.
			 * @return ConstReferenceType Const reference to the protected
			 * value.
			 */
			ConstReferenceType operator*() const noexcept;

			/**
			 * @brief Accessor to the value.
			 * @return ReferenceType Reference to the protected.
			 */
			ReferenceType operator*() noexcept;

			/// The lock that manages the lockable object.
			mutable LockType lock;

		private:
			/// The protected value.
			ReferenceType m_value;
		};

	public:
		/// Reference-to-const ValueType.
		using ConstReferenceType = const RemoveRefValueType&;
		/// Reference to ValueType.
		using ReferenceType = RemoveRefValueType&;

		/// Template type alias to an AccessImpl with non-const access
		template<template<typename> class LockType, ReadOrWrite AccessType=ReadWrite>
		using Access = AccessImpl<LockType<RemoveRefLockableType>, AccessType>;
		
		/**
		 * @brief Construct a Safe object
		 */
		Safe() = default;
		/**
		 * @brief Construct a new Safe object with default construction of
		 * the LockableType object and perfect forwarding of the other
		 * arguments to construct the ValueType object.
		 * 
		 * @tparam ValueArgs Perfect forwarding types to construct the ValueType object.
		 * @param tag Indicates that the LockableType object should be default constructed.
		 * @param valueArgs Perfect forwarding arguments to construct the ValueType object.
		 */
		template<typename... ValueArgs>
		Safe(default_construct_lockable_t, ValueArgs&&... valueArgs);
		/**
		 * @brief Construct a Safe object, perfect forwarding the first
		 * argument to construct the LockableType object and perfect forwarding
		 * the other arguments to construct the ValueType object.
		 * 
		 * @tparam LockableArg Perfect forwarding type to construct the LockableType object.
		 * @tparam ValueArgs Perfect forwarding types to construct the ValueType object.
		 * @param lockableArg Perfect forwarding argument to construct the LockableType object.
		 * @param valueArgs Perfect forwarding arguments to construct the ValueType object.
		 */
		template<typename LockableArg, typename... ValueArgs>
		Safe(LockableArg&& lockableArg, ValueArgs&&... valueArgs);

	/**
	 * @brief Creates an Access object with read-write behavior based on
	 * LockType's isShared trait (see locktraits.h). By default, the
	 * created Access object is read-write. If a specialization of
	 * safe::LockTraits exists for the LockType and defines isShared as
	 * true, then the Access is read-only.
	 * 
	 * @tparam LockType The type of the lock object that manages the
	 * lockable object.
	 * @return Access<LockType, ReadOnly> or Access<LockType, ReadWrite>
	 * The Access object used to access the value object.
	 */
		template<template<typename> class LockType = std::lock_guard>
		auto access() -> typename std::conditional<LockTraits<LockType>::isShared, Access<LockType, ReadOnly>, Access<LockType, ReadWrite>>::type;

		/**
		 * @brief Creates an Access object.
		 * 
		 * @tparam LockType The type of the lock object that manages the
	 * lockable object.
		 * @tparam AccessType ReadWrite or ReadOnly.
		 * @return Access<LockType, AccessType> The Access object used to
		 * access the value object.
		 */
		template<template<typename> class LockType, ReadOrWrite AccessType>
		Access<LockType, AccessType> access();

		/**
		 * @brief Unsafe const accessor to the value.
		 * 
		 * @return ConstReferenceType Const reference to the value.
		 */
		ConstReferenceType unsafe() const noexcept;
		/**
		 * @brief Unsafe accessor to the value.
		 * 
		 * @return ReferenceType Reference to the value.
		 */
		ReferenceType unsafe() noexcept;

		/**
		 * @brief Const accessor to the lockable object.
		 * 
		 * @return const RemoveRefLockableType& Const reference to the
		 * lockable object.
		 */
		const RemoveRefLockableType& lockable() const noexcept;

		/**
		 * @brief Accessor to the lockable object.
		 * 
		 * @return RemoveRefLockableType& Reference to the lockable object.
		 */
		RemoveRefLockableType& lockable() noexcept;

	private:
		/// The helper class instance that holds the lockable object, or a reference to it.
		MutableIfNotReferenceLockableType<LockableType> m_lockable;

		/// The value to protect.
		ValueType m_value;
	};

	/// Access with std::lock_guard behavior
	template<typename SafeType, ReadOrWrite AccessType=ReadWrite>
	using LockGuard = typename SafeType::template Access<std::lock_guard, AccessType>;
	/// Access with std::unique_lock behavior
	template<typename SafeType, ReadOrWrite AccessType=ReadWrite>
	using UniqueLock = typename SafeType::template Access<std::unique_lock, AccessType>;
}  // namespace safe

#endif /* SAFE_H_ */
