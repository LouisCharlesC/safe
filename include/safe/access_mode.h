// Copyright (c) 2019-2023 Louis-Charles Caron

// This file is part of the safe library (https://github.com/LouisCharlesC/safe).

// Use of this source code is governed by an MIT-style license that can be
// found in the LICENSE file or at https://opensource.org/licenses/MIT.

#pragma once

#include <mutex>
#if __cplusplus >= 201402L
#include <shared_mutex>
#endif // __cplusplus >= 201402L

namespace safe
{
enum class AccessMode
{
    ReadOnly,
    ReadWrite
};

// Base template: most LockTypes are not read only.
// Specialize this template if you want to force a LockType to be read only.
// If you specialize this template, consider using the pattern described in the 
// test_default_locks.cpp file to avoid ill-formed programs!
template <typename LockType> struct AccessTraits
{
    static constexpr bool IsReadOnly = false;
};
// Partial specialization for lock_guard: not read only.
template <typename MutexType> struct AccessTraits<std::lock_guard<MutexType>>
{
    static constexpr bool IsReadOnly = false;
};
// Partial specialization for unique_lock: not read only.
template <typename MutexType> struct AccessTraits<std::unique_lock<MutexType>>
{
    static constexpr bool IsReadOnly = false;
};
// Partial specialization for shared_lock: read only!
#if __cplusplus >= 201402L
template <typename MutexType> struct AccessTraits<std::shared_lock<MutexType>>
{
    static constexpr bool IsReadOnly = true;
};
#endif // __cplusplus >= 201402L
} // namespace safe