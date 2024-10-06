// Copyright (c) 2019-2022 Louis-Charles Caron

// This file is part of the safe library (https://github.com/LouisCharlesC/safe).

// Use of this source code is governed by an MIT-style license that can be
// found in the LICENSE file or at https://opensource.org/licenses/MIT.

#pragma once

#include <mutex>

namespace safe
{
namespace impl
{
// Base template defining default lock types for all mutex types.
// Specialize this template as shown in the ReadMe and tests to define your own default locks.
// This struct only uses the first template parameter (MutexType), the parameter pack is there
// only to allow one to partially specialize the template using a single template parameter. Doing
// so overrides the default types for any MutexType. See the test_default_locks.cpp file for examples.
template<typename MutexType, typename...>
struct DefaultLocks
{
    using ReadOnly = std::lock_guard<MutexType>;
    using ReadWrite = std::lock_guard<MutexType>;
};
} // namespace impl

template<typename MutexType>
using DefaultReadOnlyLockType = typename impl::DefaultLocks<MutexType>::ReadOnly;
template<typename MutexType>
using DefaultReadWriteLockType = typename impl::DefaultLocks<MutexType>::ReadWrite;
} // namespace safe