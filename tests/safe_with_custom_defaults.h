#pragma once

#include "safe/safe.h"

// Specialize DefaultLocks for any mutex types. This effectively specifies new default locks.
template<typename MutexType>
struct safe::impl::DefaultLocks<MutexType>
{
    using ReadOnly = std::lock_guard<MutexType>;
    using ReadWrite = std::unique_lock<MutexType>;
};

// Specialize DefaultLocks for a specific mutex type (here: std::timed_mutex).
template<>
struct safe::impl::DefaultLocks<std::timed_mutex>
{
    using ReadOnly = std::unique_lock<std::timed_mutex>;
    using ReadWrite = std::lock_guard<std::timed_mutex>;
};
