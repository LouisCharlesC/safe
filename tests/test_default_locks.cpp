// Copyright (c) 2023 Louis-Charles Caron

// This file is part of the safe library (https://github.com/LouisCharlesC/safe).

// Use of this source code is governed by an MIT-style license that can be
// found in the LICENSE file or at https://opensource.org/licenses/MIT.

#include "safe/safe.h"
#include "doctest/doctest.h"

#include <type_traits>

// Specialize DefaultLocks for any mutex types. This effectively specifies new default locks.
template<typename MutexType>
struct safe::DefaultLocks<MutexType>
{
    using ReadOnly = std::lock_guard<MutexType>;
    using ReadWrite = std::unique_lock<MutexType>;
};

// Specialize DefaultLocks for a specific mutex type (here: std::timed_mutex).
template<>
struct safe::DefaultLocks<std::timed_mutex>
{
    using ReadOnly = std::unique_lock<std::timed_mutex>;
    using ReadWrite = std::lock_guard<std::timed_mutex>;
};

TEST_CASE("First specialization works for all mutexes")
{
    static_assert(std::is_same<decltype(safe::Safe<int>::ReadAccess<>::lock), std::lock_guard<std::mutex>>::value, "Specialization did not work!");
    static_assert(std::is_same<decltype(safe::Safe<int>::WriteAccess<>::lock), std::unique_lock<std::mutex>>::value, "Specialization did not work!");
}

TEST_CASE("Second specialization overrides even the first one")
{
    static_assert(std::is_same<decltype(safe::Safe<int, std::timed_mutex>::ReadAccess<>::lock), std::unique_lock<std::timed_mutex>>::value, "Specialization did not work!");
    static_assert(std::is_same<decltype(safe::Safe<int, std::timed_mutex>::WriteAccess<>::lock), std::lock_guard<std::timed_mutex>>::value, "Specialization did not work!");
}
