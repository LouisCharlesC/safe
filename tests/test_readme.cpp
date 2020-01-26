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

#include "safe/safe.h"

#include "doctest/doctest.h"

#include <cassert>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

void readmeWithoutSafeExample()
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

void readmeWithSafeExample()
{
using SafeString = safe::Safe<std::string>; // type alisases will save you a lot of typing
SafeString foo; // value and mutex packaged together!
SafeString bar;
std::string baz; // now you can see that this variable has no mutex

{
	safe::WriteAccess<SafeString> fooAccess(foo); // this locks the mutex and gives you access to foo, nothing more
	*fooAccess = "Hello, World!"; // access the value using pointer semantics: * and ->
}

//std::cout << bar << std::endl; // does not compile!
std::cout << bar.unsafe() << std::endl; // unprotected access: clearly expressed!
std::cout << baz << std::endl; // all good (remember, baz has no mutex!)
}

void readmeBasicUsageWithoutSafe()
{
std::mutex mutex;
std::string value;
{
	std::lock_guard<std::mutex> lock(mutex);
	value = "42";
}
}

void readmeBasicUsageWithSafe()
{
safe::Safe<int> value;
safe::WriteAccess<safe::Safe<int>> valueAccess(value);
safe::Safe<int>::WriteAccess<> valueAccess2(value); // equivalent to the above
#if __cplusplus >= 201703L
auto valueAccess3 = value.writeAccess(); // only with C++17 and later
#endif
auto valueAccess4 = value.writeAccess<std::unique_lock>();
}

safe::Safe<int, std::mutex> valmut();
safe::Safe<int> valdef(); // equivalent to the above, as the second template parameter defaults to std::mutex
safe::Safe<int&, std::mutex> refmut();
safe::Safe<int, std::mutex&> valref();
safe::Safe<int&, std::mutex&> refref();

void readmeDefaultConstructLockableTag()
{
std::mutex aMutex;

safe::Safe<int, std::mutex> bothDefault; // mutex and value are default constructed
safe::Safe<int, std::mutex&> noDefault(aMutex, 42); // mutex and value are initialized
safe::Safe<int, std::mutex&> valueDefault(aMutex); // mutex is initialized, and value is default constructed
safe::Safe<int, std::mutex> mutexDefaultTag(safe::default_construct_mutex, 42); // mutex is default constructed, and value is initialized
safe::Safe<int, std::mutex> mutexDefaultBraces({}, 42);
}

void readmeFlexiblyConstructLock()
{
safe::Safe<int> value; // given a Safe object
value.mutex().lock(); // with the mutex already locked...
// Because the mutex is already locked, you need to pass the std::adopt_lock tag to std::lock_guard when you construct your Access object.

// Fortunately, arguments passed to WriteAccess's constructor are forwarded to the lock's constructor.
safe::WriteAccess<safe::Safe<int>> valueAccess(value, std::adopt_lock);
}

void readmeLegacy()
{
std::mutex mutex;
int value;

// Wrap the existing variables
safe::Safe<int&, std::mutex&> safeValue(mutex, value);
// do not use mutex and unsafeValue directly from here on!
}

void readmeConditionVariable()
{
std::condition_variable cv;
safe::Safe<int> value;

safe::Safe<int>::WriteAccess<std::unique_lock> valueAccess(value);
cv.wait(valueAccess.lock);
}

TEST_CASE("Readme basic usage")
{
safe::Safe<std::vector<std::string>> vec;
vec.assign(std::vector<std::string>(2, "bar")); // assign a whole new vector
auto copy = vec.copy(); // copy whole vector out of safe object
vec.writeAccess()->front() = "foo"; // replace front only

assert(vec.readAccess()->front() == "foo"); // check vec's front is "foo"
assert(copy.front() == "bar"); // check copy's front is "bar"
}