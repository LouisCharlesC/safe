// Copyright (c) 2019-2023 Louis-Charles Caron

// This file is part of the safe library (https://github.com/LouisCharlesC/safe).

// Use of this source code is governed by an MIT-style license that can be
// found in the LICENSE file or at https://opensource.org/licenses/MIT.

#pragma once

#include "access_mode.h"
#include "default_locks.h"
#include "meta.h"
#include "mutable_ref.h"

#include <type_traits>
#include <utility>

#if __cplusplus >= 201703L
#define EXPLICIT_IF_CPP17 explicit
#define EXPLICITLY_CONSTRUCT_RETURN_TYPE_IF_CPP17 ReturnType
#else
#define EXPLICIT_IF_CPP17
#define EXPLICITLY_CONSTRUCT_RETURN_TYPE_IF_CPP17
#endif

namespace safe
{
namespace impl
{
struct DefaultConstructMutex
{
};
} // namespace impl

/**
 * @brief Use this tag to default construct the mutex when constructing a Safe object.
 */
constexpr impl::DefaultConstructMutex default_construct_mutex;

/**
 * @brief Wraps a value together with a mutex.
 *
 * @tparam ValueType The type of the value to protect.
 * @tparam MutexType The type of the mutex.
 */
template <typename ValueType, typename MutexType = std::mutex> class Safe
{
  private:
    /// Type ValueType with reference removed, if present
    using RemoveRefValueType = typename std::remove_reference<ValueType>::type;
    /// Type MutexType with reference removed, if present
    using RemoveRefMutexType = typename std::remove_reference<MutexType>::type;

    /**
     * @brief Manages a mutex and gives pointer-like access to a value object.
     *
     * @tparam LockType The type of the lock object that manages the mutex, example: std::lock_guard.
     * @tparam Mode Determines the access mode of the Access object. Can be either AccessMode::ReadOnly or
     * AccessMode::ReadWrite.
     */
    template <template <typename> class LockType, AccessMode Mode> class Access
    {
        // Make sure AccessMode is ReadOnly if a read-only lock is used
        static_assert(!(AccessTraits<LockType<RemoveRefMutexType>>::IsReadOnly && Mode == AccessMode::ReadWrite),
                      "Cannot have ReadWrite access mode with ReadOnly lock. "
                      "Check the value of "
                      "AccessTraits<LockType>::IsReadOnly if it exists.");

        /// ValueType with const qualifier if AccessMode is ReadOnly.
        using ConstIfReadOnlyValueType =
            typename std::conditional<Mode == AccessMode::ReadOnly, const RemoveRefValueType, RemoveRefValueType>::type;

      public:
        /// Pointer-to-const ValueType
        using ConstPointerType = const ConstIfReadOnlyValueType *;
        /// Pointer-to-const ValueType if Mode is ReadOnly, pointer to ValueType otherwise.
        using PointerType = ConstIfReadOnlyValueType *;
        /// Reference-to-const ValueType
        using ConstReferenceType = const ConstIfReadOnlyValueType &;
        /// Reference-to-const ValueType if Mode is ReadOnly, reference to ValueType otherwise.
        using ReferenceType = ConstIfReadOnlyValueType &;

        /**
         * @brief Construct an Access object from a possibly const reference to the value object and any additionnal
         * argument needed to construct the Lock object.
         *
         * @tparam LockArgs Deduced from lockArgs.
         * @param value Reference to the value.
         * @param lockArgs Arguments needed to construct the lock object.
         */
        template <typename... OtherLockArgs>
        EXPLICIT_IF_CPP17 Access(ReferenceType value, MutexType &mutex, OtherLockArgs &&...otherLockArgs)
            : lock(mutex, std::forward<OtherLockArgs>(otherLockArgs)...), m_value(value)
        {
        }

        /**
         * @brief Construct a read-only Access object from a const safe::Safe object and any additionnal argument needed
         * to construct the Lock object.
         *
         * If needed, you can provide additionnal arguments to construct the lock object (such as std::adopt_lock). The
         * mutex from the safe::Locakble object is already passed to the lock object's constructor though, you must not
         * provide it.
         *
         * @tparam OtherLockArgs Deduced from otherLockArgs.
         * @param safe The const Safe object to give protected access to.
         * @param otherLockArgs Other arguments needed to construct the lock object.
         */
        template <typename... OtherLockArgs>
        EXPLICIT_IF_CPP17 Access(const Safe &safe, OtherLockArgs &&...otherLockArgs)
            : Access(safe.m_value, safe.m_mutex.get, std::forward<OtherLockArgs>(otherLockArgs)...)
        {
        }

        /**
         * @brief Construct a read-write Access object from a safe::Safe object and any additionnal argument needed to
         * construct the Lock object.
         *
         * If needed, you can provide additionnal arguments to construct the lock object (such as std::adopt_lock). The
         * mutex from the safe object is already passed to the lock object's constructor though, you must not provide
         * it.
         *
         * @tparam OtherLockArgs Deduced from otherLockArgs.
         * @param safe The Safe object to give protected access to.
         * @param otherLockArgs Other arguments needed to construct the lock object.
         */
        template <typename... OtherLockArgs>
        EXPLICIT_IF_CPP17 Access(Safe &safe, OtherLockArgs &&...otherLockArgs)
            : Access(safe.m_value, safe.m_mutex.get, std::forward<OtherLockArgs>(otherLockArgs)...)
        {
        }

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
         * @return ConstValueReferenceType Const reference to the protected value.
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

        /// The lock that manages the mutex.
        mutable LockType<RemoveRefMutexType> lock;

      private:
        /// The protected value.
        ReferenceType m_value;
    };

    /// Reference-to-const ValueType.
    using ConstValueReferenceType = const RemoveRefValueType &;
    /// Reference to ValueType.
    using ValueReferenceType = RemoveRefValueType &;
    /// Reference to MutexType.
    using MutexReferenceType = RemoveRefMutexType &;

    struct UseLastArgumentForMutex
    {
    };
    struct LastArgumentIsATag
    {
    };

  public:
    /// Aliases to ReadAccess and WriteAccess classes for this Safe class.
    template <template <typename> class LockType = DefaultReadOnlyLockType>
    using ReadAccess = Access<LockType, AccessMode::ReadOnly>;
    template <template <typename> class LockType = DefaultReadWriteLockType>
    using WriteAccess = Access<LockType, AccessMode::ReadWrite>;

    /**
     * @brief Construct a Safe object
     */
    Safe() = default;
    /**
     * @brief Construct a Safe object, forwarding the last argument to construct the mutex and the other arguments to
     * construct the value object. This constructor will be selected if the mutex can be constructed from the last
     * argument of the parameter pack. To avoid using the last argument to construct the mutex, add the
     * default_construct_mutex tag as last argument.
     *
     * @tparam Args Deduced from args.
     * @tparam SFINAE constraint.
     * @param args Perfect forwarding arguments to split between the value and mutex.
     */
    template <typename... Args,
              typename std::enable_if<std::is_constructible<MutexType, Last<Args...>>::value, bool>::type = true>
    explicit Safe(Args &&...args)
        : Safe(UseLastArgumentForMutex(), std::forward_as_tuple(std::forward<Args>(args)...),
               safe::impl::make_index_sequence<sizeof...(args) - 1>()) // delegate to a private constructor to split the
                                                                       // parameter pack
    {
    }
    /**
     * @brief Construct a Safe object, forwarding all arguments to construct the value object. This constructor will be
     * selected if the mutex cannot be constructed from the last argument of the parameter pack.
     *
     * @tparam Args Deduced from args.
     * @param args Perfect forwarding arguments to construct the value object.
     */
    template <typename... Args,
              typename std::enable_if<!std::is_same<const impl::DefaultConstructMutex &, Last<Args...>>::value &&
                                          !std::is_constructible<MutexType, Last<Args...>>::value,
                                      bool>::type = true>
    explicit Safe(Args &&...args) : m_mutex{}, m_value(std::forward<Args>(args)...)
    {
    }
    /**
     * @brief Construct a Safe object, forwarding all arguments but the last (the default_construct_mutex tag) to
     * construct the value object.
     *
     * @tparam Args Deduced from args.
     * @param default_construct_mutex tag.
     * @param args Perfect forwarding arguments to construct the value object.
     */
    template <typename... Args,
              typename std::enable_if<std::is_same<const impl::DefaultConstructMutex &, Last<Args...>>::value,
                                      bool>::type = true>
    explicit Safe(Args &&...args)
        : Safe(LastArgumentIsATag(), std::forward_as_tuple(std::forward<Args>(args)...),
               safe::impl::make_index_sequence<sizeof...(args) - 1>()) // delegate to a private constructor to split the
                                                                       // parameter pack
    {
    }

    /// Delete all copy/move construction/assignment, as these operations require locking the mutex under the covers.
    Safe(const Safe &) = delete;
    Safe(Safe &&) = delete;
    Safe &operator=(const Safe &) = delete;
    Safe &operator=(Safe &&) = delete;

    /**
     * @brief Lock the Safe object to get a ReadAccess object.
     *
     * @tparam Args Deduced from args.
     * @param args Perfect forwarding arguments to construct the lock object.
     */
    template <template <typename> class LockType = DefaultReadOnlyLockType, typename... LockArgs>
    ReadAccess<LockType> readLock(LockArgs &&...lockArgs) const
    {
        using ReturnType = ReadAccess<LockType>;
        return EXPLICITLY_CONSTRUCT_RETURN_TYPE_IF_CPP17{*this, std::forward<LockArgs>(lockArgs)...};
    }

    /**
     * @brief Lock the Safe object to get a WriteAccess object.
     *
     * @tparam Args Deduced from args.
     * @param args Perfect forwarding arguments to construct the lock object.
     */
    template <template <typename> class LockType = DefaultReadWriteLockType, typename... LockArgs>
    WriteAccess<LockType> writeLock(LockArgs &&...lockArgs)
    {
        using ReturnType = WriteAccess<LockType>;
        return EXPLICITLY_CONSTRUCT_RETURN_TYPE_IF_CPP17{*this, std::forward<LockArgs>(lockArgs)...};
    }

    /**
     * @brief Unsafe const accessor to the value. If you use this function, you exit the realm of safe!
     *
     * @return ConstValueReferenceType Const reference to the value object.
     */
    ConstValueReferenceType unsafe() const noexcept
    {
        return m_value;
    }
    /**
     * @brief Unsafe accessor to the value. If you use this function, you exit the realm of safe!
     *
     * @return ValueReferenceType Reference to the value object.
     */
    ValueReferenceType unsafe() noexcept
    {
        return m_value;
    }

    /**
     * @brief Accessor to the mutex.
     *
     * @return MutexReferenceType Reference to the mutex.
     */
    MutexReferenceType mutex() const noexcept
    {
        return m_mutex.get;
    }

  private:
    // The next two constructors are helper constructors to split the input arguments between the value and mutex
    // constructors
    template <typename ArgsTuple, size_t... AllButLast>
    explicit Safe(const UseLastArgumentForMutex, ArgsTuple &&args, safe::impl::index_sequence<AllButLast...>)
        : m_mutex{std::get<sizeof...(AllButLast)>(
              std::forward<ArgsTuple>(args))},                            // use the last argument to constuct the mutex
          m_value(std::get<AllButLast>(std::forward<ArgsTuple>(args))...) // and the rest to construct the value
    {
    }
    template <typename ArgsTuple, size_t... AllButLast>
    explicit Safe(const LastArgumentIsATag, ArgsTuple &&args, safe::impl::index_sequence<AllButLast...>)
        : m_mutex{}, // default construct the mutex since the last argument is a tag
          m_value(std::get<AllButLast>(std::forward<ArgsTuple>(args))...) // use the rest to construct the value
    {
    }

    /// The helper object that holds the mutable mutex, or a reference to a mutex.
    impl::MutableIfNotReference<MutexType> m_mutex;
    /// The value to protect.
    ValueType m_value;
};

/**
 * @brief Type alias for read-only Access.
 *
 * @tparam SafeType The type of Safe object to give read-only access to.
 * @tparam LockType The type of lock.
 */
template <typename SafeType, template <typename> class LockType = DefaultReadOnlyLockType>
using ReadAccess = typename SafeType::template ReadAccess<LockType>;

/**
 * @brief Type alias for read-write Access.
 *
 * @tparam SafeType The type of Safe object to give read-write access to.
 * @tparam LockType The type of lock.
 */
template <typename SafeType, template <typename> class LockType = DefaultReadWriteLockType>
using WriteAccess = typename SafeType::template WriteAccess<LockType>;
} // namespace safe

#undef EXPLICIT_IF_CPP17
#undef EXPLICITLY_CONSTRUCT_RETURN_TYPE_IF_CPP17