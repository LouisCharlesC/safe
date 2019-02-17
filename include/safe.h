/*
 * safe.h
 *
 *  Created on: Sep 21, 2018
 *      Author: lcc
 */

#ifndef SAFE_H_
#define SAFE_H_

#include "safetraits.h"

#include <mutex>
#include <type_traits>

namespace safe
{
	/**
	 * @brief Tag to use the lockable's default constructor when arguments
	 * are provided to contruct the value object.
	 */
	struct default_construct_lockable_t {};
	static constexpr default_construct_lockable_t default_construct_lockable;
	
	namespace impl
	{
		/**
		 * @brief A helper class that defines a member variable of type
		 * LockableType. The variable is declared mutable if LockableType
		 * is not a reference, the variable is not mutable if LockableType
		 * is a reference.
		 * 
		 * @tparam LockableType The type of the variable to define.
		 */
		template<typename LockableType>
		struct MutableIfNotReferenceLockableType
		{
			MutableIfNotReferenceLockableType() = default;
			template<typename LockableArg>
			MutableIfNotReferenceLockableType(LockableArg&& lockableArg):
				lockable(std::forward<LockableArg>(lockableArg))
			{}
			/// Mutable Lockable object.
			mutable LockableType lockable;
		};
		template<typename LockableType>
		struct MutableIfNotReferenceLockableType<LockableType&>
		{
			MutableIfNotReferenceLockableType(LockableType& lockable):
				lockable(lockable)
			{}
			/// Reference to a Lockable object.
			LockableType& lockable;
		};
	} // namespace impl

	/**
	 * @brief Wraps a value together with the lockable object that
	 * protects it for multi-threaded access.
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
		
	public:
		/**
		 * @brief Manages a lockable object and gives access to the value
		 * object from a Safe object.
		 * 
		 * @tparam LockType The type of the lock object that manages the
		 * lockable object, example: std::lock_guard.
		 * @tparam ReadOrWrite Determines the constness of the access to the
		 * value object, either AccessMode::ReadOnly or
		 * AccessMode::ReadWrite.
		 */
		template<template<typename> class LockType, AccessMode ReadOrWrite=LockTraits<LockType>::DefaultAccessMode>
		class Access
		{
		private:
			/// ValueType with const qualifier if AccessMode is ReadOnly.
			using ConstIfReadOnlyValueType = typename std::conditional<ReadOrWrite==AccessMode::ReadOnly, const RemoveRefValueType, RemoveRefValueType>::type;

		public:
			/// Pointer-to-const ValueType
			using ConstPointerType = const ConstIfReadOnlyValueType*;
			/// Pointer-to-const ValueType if ReadOrWrite is ReadOnly, pointer to ValueType otherwise.
			using PointerType = ConstIfReadOnlyValueType*;
			/// Reference-to-const ValueType
			using ConstReferenceType = const ConstIfReadOnlyValueType&;
			/// Reference-to-const ValueType if ReadOrWrite is ReadOnly, reference to ValueType otherwise.
			using ReferenceType = ConstIfReadOnlyValueType&;

			/**
			 * @brief Construct an Access object from a const Safe object,
			 * managing its lockable object and exposing its value object.
			 * 
			 * The lockable object from the safe object is already passed to
			 * the lock object's constructor, you must not provide it. Only
			 * provide additional constructor arguments.
			 * 
			 * @tparam OtherLockArgs Perfect forwarding types to construct
			 * the lock object.
			 * @param safe The safe object to manage.
			 * @param otherLockArgs Perfect forwarding arguments to construct
			 * the lock object.
			 */
			template<typename... OtherLockArgs>
			Access(const Safe& safe, OtherLockArgs&&... otherLockArgs) noexcept(noexcept(LockType<RemoveRefLockableType>(safe.m_lockable.lockable, std::forward<OtherLockArgs>(otherLockArgs)...)));

			/**
			 * @brief Construct an Access object from a Safe object, managing
			 * its lockable object and exposing its value object.
			 * 
			 * The lockable object from the safe object is already passed to
			 * the lock object's constructor, you must not provide it. Only
			 * provide additional constructor arguments.
			 * 
			 * @tparam OtherLockArgs Perfect forwarding types to construct
			 * the lock object other than the lockable object.
			 * @param safe The safe object to manage.
			 * @param otherLockArgs Perfect forwarding arguments to construct the lock object.
			 */
			template<typename... OtherLockArgs>
			Access(Safe& safe, OtherLockArgs&&... otherLockArgs) noexcept(noexcept(LockType<RemoveRefLockableType>(safe.m_lockable.lockable, std::forward<OtherLockArgs>(otherLockArgs)...)));

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
			mutable LockType<RemoveRefLockableType> lock;

		private:
			/// The protected value.
			ReferenceType m_value;
		};

		/// Reference-to-const ValueType.
		using ConstReferenceType = const RemoveRefValueType&;
		/// Reference to ValueType.
		using ReferenceType = RemoveRefValueType&;
		
		/**
		 * @brief Construct a Safe object
		 */
		Safe() = default;
		/**
		 * @brief Construct a Safe object with default construction of the
		 * Lockable object and perfect forwarding of the other arguments to
		 * construct the value object.
		 * 
		 * @tparam ValueArgs Perfect forwarding types to construct the value object.
		 * @param tag Indicates that the lockable object should be default constructed.
		 * @param valueArgs Perfect forwarding arguments to construct the value object.
		 */
		template<typename... ValueArgs>
		Safe(default_construct_lockable_t, ValueArgs&&... valueArgs);
		/**
		 * @brief Construct a Safe object, perfect forwarding the first
		 * argument to construct the lockable object and perfect forwarding
		 * the other arguments to construct the value object.
		 * 
		 * @tparam LockableArg Perfect forwarding type to construct the lockable object.
		 * @tparam ValueArgs Perfect forwarding types to construct the value object.
		 * @param lockableArg Perfect forwarding argument to construct the lockable object.
		 * @param valueArgs Perfect forwarding arguments to construct the value object.
		 */
		template<typename LockableArg, typename... ValueArgs>
		Safe(LockableArg&& lockableArg, ValueArgs&&... valueArgs);

		/**
		 * @brief Create an Access object with read-only access mode.
		 *
		 * The lockable object from the safe object is already passed to
		 * the lock object's constructor, you must not provide it. Only
		 * provide additional constructor arguments.
		 * 
		 * @tparam LockType The type of the lock object that manages the
		 * lockable object.
		 * @tparam OtherLockArgs Perfect forwarding types to construct the
		 * lock object.
		 * @param otherLockArgs Perfect forwarding arguments to construct
		 * the lock object.
		 * @return Access<LockType, AccessMode::ReadOnly> The Access object
		 * used to access the value object.
		 */
		template<template<typename> class LockType = std::lock_guard, typename... OtherLockArgs>
		Access<LockType, AccessMode::ReadOnly> access(OtherLockArgs&&... otherLockArgs) const;

	/**
	 * @brief Create an Access object with read-or-write access mode
	 * based on LockType's DefaultAccessMode trait (see safetraits.h).
	 * 
	 * If no specialization of safe::LockTraits exists for LockType, the
	 * access is read-write. If such a specialization exists, the access
	 * mode is defined by the DefaultAccessMode variable defined in the
	 * specialization.
	 * The lockable object from the safe object is already passed to
	 * the lock object's constructor, you must not provide it. Only
	 * provide additional constructor arguments.
	 * 
	 * @tparam LockType The type of the lock object that manages the
	 * lockable object.
	 * @tparam OtherLockArgs Perfect forwarding types to construct the
	 * lock object.
	 * @param otherLockArgs Perfect forwarding arguments to construct
	 * the lock object.
	 * @return Access<LockType, LockTraits<LockType>::DefaultAccessMode>
	 * The Access object used to access the value object.
	 */

		template<template<typename> class LockType = std::lock_guard, typename... OtherLockArgs>
		Access<LockType, LockTraits<LockType>::DefaultAccessMode> access(OtherLockArgs&&... otherLockArgs);

		/**
		 * @brief Create an Access object.
		 * 
		 * The lockable object from the safe object is already passed to
		 * the lock object's constructor, you must not provide it. Only
		 * provide additional constructor arguments.
		 * 
		 * @tparam LockType The type of the lock object that manages the
		 * lockable object.
		 * @tparam ReadOrWrite Determines the constness of the access to the
		 * value object, can be either AccessMode::ReadOnly or
		 * AccessMode::ReadWrite.
		 * @tparam OtherLockArgs Perfect forwarding types to construct the
		 * lock object.
		 * @param otherLockArgs Perfect forwarding arguments to construct
		 * the lock object.
		 * @return Access<LockType, AccessMode> The Access object used to
		 * access the value object.
		 */
		template<template<typename> class LockType, AccessMode ReadOrWrite, typename... OtherLockArgs>
		Access<LockType, ReadOrWrite> access(OtherLockArgs&&... otherLockArgs);

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
		/// The helper class instance that holds the mutable lockable object, or a reference to it.
		impl::MutableIfNotReferenceLockableType<LockableType> m_lockable;

		/// The value to protect.
		ValueType m_value;
	};
}  // namespace safe

#endif /* SAFE_H_ */
