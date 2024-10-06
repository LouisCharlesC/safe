// Copyright (c) 2023 Louis-Charles Caron

// This file is part of the safe library (https://github.com/LouisCharlesC/safe).

// Use of this source code is governed by an MIT-style license that can be
// found in the LICENSE file or at https://opensource.org/licenses/MIT.

// To avoid any problem with specialized templates not being visible from every compilation unit, you should use the
// pattern shown here: create your own header file in which you include safe and define your template specializations.
// Only ever include your own header in your project.
#include "safe_with_custom_defaults.h"

#include <doctest/doctest.h>
#include <type_traits>

TEST_CASE("First specialization works for all mutexes")
{
    static_assert(std::is_same<safe::DefaultReadOnlyLockType<std::mutex>, std::lock_guard<std::mutex>>::value, "Specialization did not work!");
    static_assert(std::is_same<safe::DefaultReadWriteLockType<std::mutex>, std::unique_lock<std::mutex>>::value, "Specialization did not work!");

    static_assert(std::is_same<safe::DefaultReadOnlyLockType<std::recursive_mutex>, std::lock_guard<std::recursive_mutex>>::value, "Specialization did not work!");
    static_assert(std::is_same<safe::DefaultReadWriteLockType<std::recursive_mutex>, std::unique_lock<std::recursive_mutex>>::value, "Specialization did not work!");
}

TEST_CASE("Second specialization overrides even the first one")
{
    static_assert(std::is_same<safe::DefaultReadOnlyLockType<std::timed_mutex>, std::unique_lock<std::timed_mutex>>::value, "Specialization did not work!");
    static_assert(std::is_same<safe::DefaultReadWriteLockType<std::timed_mutex>, std::lock_guard<std::timed_mutex>>::value, "Specialization did not work!");
}
