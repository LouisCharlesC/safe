// Copyright (c) 2019-2022 Louis-Charles Caron

// This file is part of the safe library (https://github.com/LouisCharlesC/safe).

// Use of this source code is governed by an MIT-style license that can be
// found in the LICENSE file or at https://opensource.org/licenses/MIT.

#pragma once

#include <mutex>

namespace safe
{
    template<typename MutexType, typename...>
    struct DefaultLocks
    {
        using ReadOnly = std::lock_guard<MutexType>;
        using ReadWrite = std::lock_guard<MutexType>;
    };

    template<typename MutexType>
    using DefaultReadOnlyLock = typename DefaultLocks<MutexType>::ReadOnly;
    template<typename MutexType>
    using DefaultReadWriteLock = typename DefaultLocks<MutexType>::ReadWrite;
} // namespace safe