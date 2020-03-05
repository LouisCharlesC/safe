/**
 * @file test_readme.cpp
 * @author L.-C. C.
 * @brief 
 * @version 0.1
 * @date 2019-04-18
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include <safe/safe.h>

#include <doctest/doctest.h>

#include <cassert>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

TEST_CASE("Readme without safe example")
{
std::string foo; // do I need to lock a mutex to safely access this variable ?
std::string bar;
std::string baz; // what about this one ?
std::mutex fooMutex; // don't forget to change the name of this variable if foo's name changes!
std::mutex barMutex;

{
	std::lock_guard<std::mutex> lock(fooMutex); // is this the right mutex for what I am about to do ?
	foo = "Hello, World!"; // I access foo here, but I could very well access bar, yet barMutex is not locked!
}

std::cout << bar << std::endl; // unprotected access, is this intended ?
std::cout << baz << std::endl; // what about this access ?
}

TEST_CASE("Readme with safe example")
{
using SafeString = safe::Safe<std::string>; // type aliases will save you a lot of typing
SafeString safeFoo; // std::string and mutex packaged together!
SafeString safeBar;
std::string baz; // now you can see that this variable has no mutex

{
	safe::WriteAccess<SafeString> foo(safeFoo); // this locks the mutex and gives you access to foo, nothing more
	*foo = "Hello, World!"; // access the value using pointer semantics: * and ->
}

//std::cout << safeBar << std::endl; // does not compile!
std::cout << safeBar.unsafe() << std::endl; // unprotected access: clearly expressed!
std::cout << baz << std::endl; // all good (remember, baz has no mutex!)
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
auto value = safeValue.writeAccess(); // only with C++17 and later
CHECK_EQ(&*value, &safeValue.unsafe());
CHECK_EQ(*value, 42);
}
#endif
{
auto value = safeValue.writeAccess<std::unique_lock>(); // ok even pre-C++17
CHECK_EQ(&*value, &safeValue.unsafe());
CHECK_EQ(value.lock.mutex(), &safeValue.mutex());
CHECK_EQ(*value, 42);
}
}

TEST_CASE("Readme one liners")
{
safe::Safe<int> safeValue;
*safeValue.writeAccess() = 42;
CHECK_EQ(safeValue.unsafe(), 42);
{
int copy = *safeValue.readAccess();
CHECK_EQ(copy, 42);
}
{
int copy = *safeValue.writeAccess(); // this also works...
// *safeValue.readAccess() = 42; // but this obviously doesn't!
CHECK_EQ(copy, 42);
}
safeValue.assign(43);
CHECK_EQ(safeValue.unsafe(), 43);
{
safeValue.assign(42);
CHECK_EQ(safeValue.unsafe(), 42);
int copy = safeValue.copy();
CHECK_EQ(copy, 42);
}
}

TEST_CASE("Readme ref and non ref")
{
std::mutex mutex;
int value;
safe::Safe<int, std::mutex> valmut;
safe::Safe<int> valdef; // equivalent to the above, as the second template parameter defaults to std::mutex
safe::Safe<int&, std::mutex> refmut(safe::default_construct_mutex, value);
safe::Safe<int, std::mutex&> valref(mutex, 42);
safe::Safe<int&, std::mutex&> refref(mutex, value);
CHECK_EQ(&refmut.unsafe(), &value);
CHECK_EQ(valref.unsafe(), 42);
CHECK_EQ(&valref.mutex(), &mutex);
CHECK_EQ(&refref.unsafe(), &value);
CHECK_EQ(&refref.mutex(), &mutex);
}

TEST_CASE("Readme default construct lockable tag")
{
std::mutex mutex;
safe::Safe<int, std::mutex> bothDefault; // mutex and value are default constructed
safe::Safe<int, std::mutex&> noDefault(mutex, 42); // mutex and value are initialized
safe::Safe<int, std::mutex&> valueDefault(mutex); // mutex is initialized, and value is default constructed
safe::Safe<int, std::mutex> mutexDefaultTag(safe::default_construct_mutex, 42); // mutex is default constructed, and value is initialized
safe::Safe<int, std::mutex> mutexDefaultBraces({}, 42);
CHECK_EQ(noDefault.unsafe(), 42);
CHECK_EQ(&noDefault.mutex(), &mutex);
CHECK_EQ(&valueDefault.mutex(), &mutex);
CHECK_EQ(mutexDefaultTag.unsafe(), 42);
CHECK_EQ(mutexDefaultBraces.unsafe(), 42);
}

TEST_CASE("Readme flexibly construct lock")
{
safe::Safe<int> safeValue; // given a Safe object
std::cout << "Before lock" << std::endl;
safeValue.mutex().lock(); // with the mutex already locked...
std::cout << "After lock" << std::endl;
// Because the mutex is already locked, you need to pass the std::adopt_lock tag to std::lock_guard when you construct your Access object.

// Fortunately, arguments passed to WriteAccess's constructor are forwarded to the lock's constructor.
{
std::cout << "Before adopt" << std::endl;
safe::WriteAccess<safe::Safe<int>> value(safeValue, std::adopt_lock);
std::cout << "After adopt" << std::endl;
CHECK_EQ(&*value, &safeValue.unsafe());
}
safeValue.mutex().lock();
{
safe::Safe<int>::WriteAccess<> value(safeValue, std::adopt_lock);
CHECK_EQ(&*value, &safeValue.unsafe());
}
CHECK_EQ(safeValue.mutex().try_lock(), true);
#if __cplusplus >= 201703L
{
auto value = safeValue.writeAccess(std::adopt_lock);
CHECK_EQ(&*value, &safeValue.unsafe());
}
CHECK_EQ(safeValue.mutex().try_lock(), true);
#endif
{
auto value = safeValue.writeAccess<std::unique_lock>(std::adopt_lock);
CHECK_EQ(&*value, &safeValue.unsafe());
CHECK_EQ(value.lock.mutex(), &safeValue.mutex());
}
CHECK_EQ(safeValue.mutex().try_lock(), true);
}

TEST_CASE("Readme legacy")
{
std::mutex lousyMutex;
int unsafeValue;

// Wrap the existing variables
safe::Safe<int&, std::mutex&> safeValue(lousyMutex, unsafeValue);
// do not use lousyMutex and unsafeValue directly from here on!
}

TEST_CASE("Readme condition variable")
{
std::condition_variable cv;
safe::Safe<int> safeValue;
safe::Safe<int>::WriteAccess<std::unique_lock> value(safeValue);
cv.wait(value.lock, [](){return true;});
CHECK_EQ(&*value, &safeValue.unsafe());
CHECK_EQ(value.lock.mutex(), &safeValue.mutex());
}