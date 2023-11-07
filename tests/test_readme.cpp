// Copyright (c) 2019-2023 Louis-Charles Caron

// This file is part of the safe library (https://github.com/LouisCharlesC/safe).

// Use of this source code is governed by an MIT-style license that can be
// found in the LICENSE file or at https://opensource.org/licenses/MIT.

#include "safe/safe.h"

#include <doctest/doctest.h>

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <string>

TEST_CASE("Readme without safe example")
{
std::string foo; // do I need to lock a mutex to safely access this variable ?
std::string bar;
std::string baz; // what about this one ?
std::mutex fooMutex; // don't forget to change the name of this variable if foo's name changes!
std::mutex barMutex;

{
	std::lock_guard<std::mutex> lock(fooMutex); // is this the right mutex for what I am about to do ?
	bar = "Hello, World!"; // Hmm, did I just to something wrong ?
}

std::cout << bar << std::endl; // unprotected access, is this intended ?
std::cout << baz << std::endl; // what about this access ?
}

TEST_CASE("Readme with safe example")
{
    using SafeString = safe::Safe<std::string>; // type aliases will save you a lot of typing
    SafeString safeFoo;                         // std::string and mutex packaged together!
    SafeString safeBar;
    std::string baz; // now you can see that this variable has no mutex

    {
        safe::WriteAccess<SafeString> foo(safeFoo); // this locks the mutex and gives you access to foo, nothing more
        *foo = "Hello, World!";                     // access the value using pointer semantics: * and ->
    }

    // std::cout << safeBar << std::endl; // does not compile!
    std::cout << safeBar.unsafe() << std::endl; // unprotected access: clearly expressed!
    std::cout << baz << std::endl;              // all good this is just a string!
}

TEST_CASE("Readme basic usage without safe")
{
    std::mutex mutex;
    int value;
    {
        std::lock_guard<std::mutex> lock(mutex);
        value = 42;
    }
}

TEST_CASE("Readme basic usage with safe")
{
    safe::Safe<int> safeValue;
    {
        safe::WriteAccess<safe::Safe<int>> value(safeValue);
        *value = 42;
        CHECK_EQ(&*value, &safeValue.unsafe());
        CHECK_EQ(safeValue.unsafe(), 42);
    }
    {
        safe::Safe<int>::WriteAccess<> value(safeValue); // equivalent to the above
        CHECK_EQ(&*value, &safeValue.unsafe());
        CHECK_EQ(*value, 42);
    }
#if __cplusplus >= 201703L
    {
        auto value = safeValue.writeLock(); // only with C++17 and later
        CHECK_EQ(&*value, &safeValue.unsafe());
        CHECK_EQ(*value, 42);
    }
#endif
    {
        auto value = safeValue.writeLock<std::unique_lock>(); // ok even pre-C++17
        CHECK_EQ(&*value, &safeValue.unsafe());
        CHECK_EQ(value.lock.mutex(), &safeValue.mutex());
        CHECK_EQ(*value, 42);
    }
}

TEST_CASE("Readme one liners")
{
    safe::Safe<int> safeValue;
    *safeValue.writeLock() = 42;
    CHECK_EQ(safeValue.unsafe(), 42);
    {
        int copy = *safeValue.readLock();
        CHECK_EQ(copy, 42);
    }
    {
        int copy = *safeValue.writeLock(); // this also works...
        // *safeValue.readAccess() = 42; // but this obviously doesn't!
        CHECK_EQ(copy, 42);
    }
}

TEST_CASE("Readme ref and non ref")
{
    std::mutex mutex;
    int value;
    safe::Safe<int, std::mutex> valmut;
    safe::Safe<int> valdef; // equivalent to the above, as the second template parameter defaults to std::mutex
    safe::Safe<int &, std::mutex> refmut(value, safe::default_construct_mutex);
    safe::Safe<int, std::mutex &> valref(42, mutex);
    safe::Safe<int &, std::mutex &> refref(value, mutex);
    CHECK_EQ(&refmut.unsafe(), &value);
    CHECK_EQ(valref.unsafe(), 42);
    CHECK_EQ(&valref.mutex(), &mutex);
    CHECK_EQ(&refref.unsafe(), &value);
    CHECK_EQ(&refref.mutex(), &mutex);
}

TEST_CASE("Readme default construct mutex tag")
{
    std::mutex mutex;
    safe::Safe<int, std::mutex> bothDefault;            // mutex and value are default constructed
    safe::Safe<int, std::mutex &> noDefault(42, mutex); // mutex and value are initialized
    safe::Safe<int, std::mutex &> valueDefault(mutex);  // mutex is initialized, and value is default constructed
    safe::Safe<int, std::mutex> mutexDefault(42); // mutex is default constructed, and value is initialized
    safe::Safe<int, std::mutex> mutexDefaultTag(42, safe::default_construct_mutex); // mutex is default constructed, and value is initialized
    CHECK_EQ(noDefault.unsafe(), 42);
    CHECK_EQ(&noDefault.mutex(), &mutex);
    CHECK_EQ(&valueDefault.mutex(), &mutex);
    CHECK_EQ(mutexDefaultTag.unsafe(), 42);
}

TEST_CASE("Readme flexibly construct lock")
{
    safe::Safe<int> safeValue; // given a Safe object
    safeValue.mutex().lock();  // with the mutex already locked...
    // Because the mutex is already locked, you need to pass the std::adopt_lock tag to std::lock_guard when you
    // construct your Access object.

    // Fortunately, arguments passed to WriteAccess's constructor are forwarded to the lock's constructor.
    {
        safe::WriteAccess<safe::Safe<int>> value(safeValue, std::adopt_lock);
        CHECK_EQ(&*value, &safeValue.unsafe());
    }

    safeValue.mutex().lock();
    {
        safe::Safe<int>::WriteAccess<> value(safeValue, std::adopt_lock);
        CHECK_EQ(&*value, &safeValue.unsafe());
    }

#if __cplusplus >= 201703L
    safeValue.mutex().lock();
    {
        auto value = safeValue.writeLock(std::adopt_lock);
        CHECK_EQ(&*value, &safeValue.unsafe());
    }
#endif

    safeValue.mutex().lock();
    {
        auto value = safeValue.writeLock<std::unique_lock>(std::adopt_lock);
        CHECK_EQ(&*value, &safeValue.unsafe());
        CHECK_EQ(value.lock.mutex(), &safeValue.mutex());
    }
}

TEST_CASE("Readme legacy")
{
    std::mutex lousyMutex;
    int unsafeValue;

    // Wrap the existing variables
    safe::Safe<int &, std::mutex &> safeValue(unsafeValue, lousyMutex);
    // do not use lousyMutex and unsafeValue directly from here on!
}

TEST_CASE("Readme condition variable")
{
    std::condition_variable cv;
    safe::Safe<int> safeValue;
    safe::Safe<int>::WriteAccess<std::unique_lock> value(safeValue);
    cv.wait(value.lock, []() { return true; });
    CHECK_EQ(&*value, &safeValue.unsafe());
    CHECK_EQ(value.lock.mutex(), &safeValue.mutex());
}