// Copyright (c) 2019-2022 Louis-Charles Caron

// This file is part of the safe library (https://github.com/LouisCharlesC/safe).

// Use of this source code is governed by an MIT-style license that can be
// found in the LICENSE file or at https://opensource.org/licenses/MIT.

#pragma once

#include <mutex>

namespace safe
{
    // Base template defining default lock types for all mutex types.
    // Specialize this template as shown in the ReadMe and tests to define your own default locks.
    template<typename MutexType, typename...>
    struct DefaultLocks
    {
        using ReadOnly = std::lock_guard<MutexType>;
        using ReadWrite = std::lock_guard<MutexType>;
    };

    template<typename MutexType>
    using DefaultReadOnlyLockType = typename DefaultLocks<MutexType>::ReadOnly;
    template<typename MutexType>
    using DefaultReadWriteLockType = typename DefaultLocks<MutexType>::ReadWrite;
} // namespace safe