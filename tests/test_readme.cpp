// Copyright (c) 2019-2023 Louis-Charles Caron

// This file is part of the safe library (https://github.com/LouisCharlesC/safe).

// Use of this source code is governed by an MIT-style license that can be
// found in the LICENSE file or at https://opensource.org/licenses/MIT.

#include "safe/safe.h"

#include <doctest/doctest.h>

#include <cassert>
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
#include "safe/safe.h"
safe::Safe<std::string> safeFoo; // std::string and mutex packaged together!
safe::Safe<std::string> safeBar;
std::string baz; // now you can see that this variable has no mutex

{
	safe::WriteAccess<safe::Safe<std::string>> foo(safeFoo); // this locks the mutex and gives you access to foo
	*foo = "Hello, World!"; // access the value using pointer dereference: * and ->
}

std::cout << safeBar.unsafe() << std::endl; // unprotected access: clearly expressed!
std::cout << baz << std::endl; // all good this is just a string!
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
safe::Safe<std::pair<int,int>, std::mutex> bothDefault;            // mutex and value are default constructed
safe::Safe<std::pair<int,int>, std::mutex &> noDefault(42, 24, mutex); // mutex and value are initialized
safe::Safe<std::pair<int,int>, std::mutex &> valueDefault(mutex);  // mutex is initialized, and value is default constructed
safe::Safe<std::pair<int,int>, std::mutex> mutexDefault(42, 24); // mutex is default constructed, and value is initialized
safe::Safe<std::pair<int,int>, std::mutex> mutexDefaultTag(42, 24, safe::default_construct_mutex); // mutex is default constructed, and value is initialized
CHECK_EQ(noDefault.unsafe(), std::pair<int,int>(42, 24));
CHECK_EQ(&noDefault.mutex(), &mutex);
CHECK_EQ(&valueDefault.mutex(), &mutex);
CHECK_EQ(mutexDefaultTag.unsafe(), std::pair<int,int>(42, 24));
}

TEST_CASE("Readme equivalent ways to construct lock")
{
safe::Safe<int> safeValue;
{
const safe::ReadAccess<safe::Safe<int>> value(safeValue);
}
{
const safe::Safe<int>::ReadAccess<> value(safeValue); // the empty <> are unfortunately necessary.
}
#if __cplusplus >= 201703L
{
const auto value = safeValue.readLock(); // nicer, but only with C++17 and later
}
#endif
{
    const auto value = safeValue.readLock<std::unique_lock>(); // ok even pre-C++17, using a std::unique_lock rather than the default std::lock_guard
}
{
    const safe::Safe<int>::ReadAccess<std::unique_lock> value(safeValue); // equivalent to the above
}
// All of the above exist in read-write versions, simply replace "read" by "write" (and remove the const).
}

TEST_CASE("Readme already locked mutex")
{
std::mutex aLockedMutex;
aLockedMutex.lock();
safe::Safe<int, std::mutex&> safeValue(aLockedMutex); // given a Safe object with an already locked mutex.

// Because the mutex is already locked, you need to pass the std::adopt_lock tag to std::lock_guard when you construct your Access object.
// No matter how you get your Access objects, you can pass arguments to the lock's constructor.
// Again, all the lines below are equivalent:
{
safe::WriteAccess<safe::Safe<int, std::mutex&>> value(safeValue, std::adopt_lock);
}
aLockedMutex.lock();
{
safe::Safe<int, std::mutex&>::WriteAccess<> value(safeValue, std::adopt_lock);
}
#if __cplusplus >= 201703L
aLockedMutex.lock();
{
auto value = safeValue.writeLock(std::adopt_lock); // again, only in C++17
}
#endif
aLockedMutex.lock();
{
auto value = safeValue.writeLock<std::unique_lock>(std::adopt_lock); // using a std::unique_lock still works
}
}

TEST_CASE("Readme timed mutex")
{
// Here's a safe int using a timed_mutex
safe::Safe<int, std::timed_mutex> safeValue;

// Pass in a time-out when you lock the mutex, and it will try to lock for that duration!
// safe does not do anything here, std::unique_lock does it all. safe simply forwards any argument
// it gets, and when std::unique_lock receives a std::duration, it try to lock before giving up.
auto value = safeValue.writeLock<std::unique_lock>(std::chrono::seconds(1));
// You can reach the lock through the Access object and check whether or not the locking was successful:
assert(value.lock.owns_lock());
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
