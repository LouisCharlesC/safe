// Copyright(c) 2019-2022 Louis-Charles Caron

// This file is part of the safe library.

// (https://github.com/LouisCharlesC/safe). Use of this source code is governed by an MIT-style license that can be
// found in the LICENSE file or at https://opensource.org/licenses/MIT.

#pragma once

#include <mutex>

namespace safe
{
// These are the default mutex and lock types used in safe. Feel free to change them to suit your needs.
using DefaultMutex = std::mutex;
template <typename MutexType> using DefaultReadOnlyLock = std::lock_guard<MutexType>;
template <typename MutexType> using DefaultReadWriteLock = std::lock_guard<MutexType>;
} // namespace safe