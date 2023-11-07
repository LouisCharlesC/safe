// Copyright (c) 2018-2023 Louis-Charles Caron

// This file is part of the safe library (https://github.com/LouisCharlesC/safe).

// Use of this source code is governed by an MIT-style license that can be
// found in the LICENSE file or at https://opensource.org/licenses/MIT.

#include "safe/safe.h"

#include <doctest/doctest.h>

#include <mutex>

TEST_CASE("If mutex is constructible, construct with the last argument even it if could construct the value")
{
    safe::Safe<int, int> weirdSafeInt(42);
    CHECK_EQ(weirdSafeInt.unsafe(), 0);
    CHECK_EQ(weirdSafeInt.mutex(), 42);
}

TEST_CASE("If tag is passed, do not use the argument to construct the mutex")
{
    safe::Safe<int, int> weirdSafeInt(42, safe::default_construct_mutex);
    CHECK_EQ(weirdSafeInt.unsafe(), 42);
    CHECK_EQ(weirdSafeInt.mutex(), 0);
}

TEST_CASE("If tag is passed, do not use the argument to construct the mutex")
{
    safe::Safe<int, std::mutex> weirdSafeInt(42);
    CHECK_EQ(weirdSafeInt.unsafe(), 42);
}
