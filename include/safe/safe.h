/**
 * @file safe.h
 * @author L.-C. C.
 * @brief 
 * @version 0.1
 * @date 2018-09-21
 * 
 * @copyright Copyright (c) 2018
 * 
 */

#pragma once

#include "safetraits.h"

#include <mutex>
#include <type_traits>
#include <utility>

namespace safe
{
	/**
	 * @brief Tag to use the lockable's default constructor when arguments
	 * are provided to contruct the value object.
	 */
	struct default_construct_lockable_t {};
	static constexpr default_construct_lockable_t default_construct_lockable;
	
	namespace trick
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
	} // namespace trick

	/**
	 * @brief Wraps a value together with the lockable object that
	 * protects it for multi-threaded access.
	 * 
	 * @tparam ValueType The type of the value to protect.
	 * @tparam LockableType The type of the lockable object. Use a shared
	 * lockable if possible.
	 * @tparam DefaultReadOnlyLock The lock type to use as default for
	 * read only accesses. Use a shared lock if possible.
	 */
	template<typename ValueType, typename LockableType = std::mutex, template<typename> class DefaultReadOnlyLock = std::lock_guard>
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
		 * @tparam Mode Determines the access mode of the Access
		 * object. Can be either AccessMode::ReadOnly or
		 * AccessMode::ReadWrite. Default value depends on LockType's
		 * IsReadOnly trait (see safetraits.h). If no specialization of
		 * safe::LockTraits exists for LockType, the access is ReadWrite.
		 * If such a specialization exists, the access mode depends on the
		 * IsReadOnly variable defined in the specialization. If IsReadOnly
		 * is true, access mode is ReadOnly, otherwise, it is ReadWrite.
		 */
		template<template<typename> class LockType = std::lock_guard, AccessMode Mode = LockTraits<LockType>::IsReadOnly ? AccessMode::ReadOnly : AccessMode::ReadWrite>
		class Access
		{
			static_assert(!(LockTraits<LockType>::IsReadOnly && Mode==AccessMode::ReadWrite), "Cannot have ReadWrite access mode with ReadOnly lock. Check the value of LockTraits<LockType>::IsReadOnly if it exists.");

		private:
			/// ValueType with const qualifier if AccessMode is ReadOnly.
			using ConstIfReadOnlyValueType = typename std::conditional<Mode==AccessMode::ReadOnly, const RemoveRefValueType, RemoveRefValueType>::type;

		public:
			/// Pointer-to-const ValueType
			using ConstPointerType = const ConstIfReadOnlyValueType*;
			/// Pointer-to-const ValueType if Mode is ReadOnly, pointer to ValueType otherwise.
			using PointerType = ConstIfReadOnlyValueType*;
			/// Reference-to-const ValueType
			using ConstReferenceType = const ConstIfReadOnlyValueType&;
			/// Reference-to-const ValueType if Mode is ReadOnly, reference to ValueType otherwise.
			using ReferenceType = ConstIfReadOnlyValueType&;

			/**
			 * @brief Construct an Access object from a possibly const
			 * reference to the value object and any argument needed to
			 * construct the Lock object.
			 * 
			 * @tparam LockArgs Deduced from lockArgs.
			 * @param value Reference to the value.
			 * @param lockArgs Other arguments needed to construct the lock
			 * object.
			 */
			template<typename... LockArgs>
			Access(ReferenceType value, LockArgs&&... lockArgs):
				lock(std::forward<LockArgs>(lockArgs)...),
				m_value(value)
			{}

			/**
			 * @brief Const accessor to the value.
			 * @return ConstPointerType Const pointer to the protected value.
			 */
			ConstPointerType operator->() const noexcept
			{
				return &m_value;
			}

			/**
			 * @brief Accessor to the value.
			 * @return ValuePointerType Pointer to the protected value.
			 */
			PointerType operator->() noexcept
			{
				return &m_value;
			}

			/**
			 * @brief Const accessor to the value.
			 * @return ConstValueReferenceType Const reference to the protected
			 * value.
			 */
			ConstReferenceType operator*() const noexcept
			{
				return m_value;
			}

			/**
			 * @brief Accessor to the value.
			 * @return ValueReferenceType Reference to the protected.
			 */
			ReferenceType operator*() noexcept
			{
				return m_value;
			}

			/// The lock that manages the lockable object.
			mutable LockType<RemoveRefLockableType> lock;

		private:
			/// The protected value.
			ReferenceType m_value;
		};

	private:
		/// Reference-to-const ValueType.
		using ConstValueReferenceType = const RemoveRefValueType&;
		/// Reference to ValueType.
		using ValueReferenceType = RemoveRefValueType&;
		/// Reference to LockableType.
		using LockableReferenceType = RemoveRefLockableType&;

	public:
		template<template<typename> class LockType=DefaultReadOnlyLock, typename... OtherLockArgs>
		using ReadAccess = Access<LockType, AccessMode::ReadOnly, OtherLockArgs...>;
		template<template<typename> class LockType=std::lock_guard, typename... OtherLockArgs>
		using WriteAccess = Access<LockType, AccessMode::ReadWrite, OtherLockArgs...>;
		
		/**
		 * @brief Construct a Safe object
		 */
		Safe() = default;

		// Delete all copy and move operations, as they sometimes require protected access.
		Safe(const Safe&) = delete;
		Safe(Safe&&) = delete;
		Safe& operator =(const Safe&) = delete;
		Safe& operator =(Safe&&) = delete;

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
		explicit Safe(default_construct_lockable_t, ValueArgs&&... valueArgs):
			m_lockable(),
			m_value(std::forward<ValueArgs>(valueArgs)...)
		{}
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
		explicit Safe(LockableArg&& lockableArg, ValueArgs&&... valueArgs):
			m_lockable(std::forward<LockableArg>(lockableArg)),
			m_value(std::forward<ValueArgs>(valueArgs)...)
		{}

		/**
		 * @brief Create an Access object with read-only access mode.
		 *
		 * @tparam LockType The type of the lock object that manages the
		 * lockable object.
		 * @tparam OtherLockArgs Deduced from otherLockArgs.
		 * @param otherLockArgs Other arguments needed to construct the
		 * lock object (apart from the lockable object).
		 * @return Access<LockType, AccessMode::ReadOnly> The Access
		 * object.
		 */
		template<template<typename> class LockType=DefaultReadOnlyLock, typename... OtherLockArgs>
		ReadAccess<LockType, OtherLockArgs...> readAccess(OtherLockArgs&&... otherLockArgs) const
		{
			return {m_value, m_lockable.lockable, std::forward<OtherLockArgs>(otherLockArgs)...};
		}

		/**
		 * @brief Create an Access object with read-write access mode.
		 * 
		 * @tparam LockType The type of the lock object that manages the
		 * lockable object.
		 * @tparam OtherLockArgs Deduced from otherLockArgs.
		 * @param otherLockArgs Other arguments needed to construct the
		 * lock object (apart from the lockable object).
		 * @return Access<LockType, AccessMode::ReadWrite> The Access object.
		 */
		template<template<typename> class LockType=std::lock_guard, typename... OtherLockArgs>
		WriteAccess<LockType, OtherLockArgs...> writeAccess(OtherLockArgs&&... otherLockArgs)
		{
			return {m_value, m_lockable.lockable, std::forward<OtherLockArgs>(otherLockArgs)...};
		}

		/**
		 * @brief Unsafe const accessor to the value.
		 * 
		 * @return ConstValueReferenceType Const reference to the value.
		 */
		ConstValueReferenceType unsafe() const noexcept
		{
			return m_value;
		}
		/**
		 * @brief Unsafe accessor to the value.
		 * 
		 * @return ValueReferenceType Reference to the value.
		 */
		ValueReferenceType unsafe() noexcept
		{
			return m_value;
		}

		/**
		 * @brief Accessor to the lockable object.
		 * 
		 * @return LockableReferenceType Reference to the
		 * lockable object.
		 */
		LockableReferenceType lockable() const noexcept
		{
			return m_lockable.lockable;
		}

	private:
		/// The helper object that holds the mutable lockable object, or a reference to the lockable object.
		trick::MutableIfNotReferenceLockableType<LockableType> m_lockable;
		/// The value to protect.
		ValueType m_value;
	};
}  // namespace safe
