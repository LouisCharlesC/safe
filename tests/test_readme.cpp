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

#include "safe/lockable.h"
#include "safe/safe.h"

#include "gtest/gtest.h"

#include <cassert>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

void readmeWithoutSafeExample()
{
std::mutex fooMutex;
std::mutex barMutex;
std::string foo; // <-- do I need to lock a mutex to safely access this variable ?

{
	std::lock_guard<std::mutex> lock(fooMutex); // <-- is this the right mutex ?
	foo = "Hello, World!";
}

std::cout << foo << std::endl; // <-- unprotected access, is this intended ?
}

void readmeWithSafeExample()
{
using LockableString = safe::Lockable<std::string>; // type alisases will save you a lot of typing
std::mutex barMutex;
LockableString foo; // <-- value and mutex packaged together!

{
	safe::WriteAccess<LockableString> fooAccess(foo); // <-- right mutex: guaranteed!

	*fooAccess = "Hello, World!"; // access the value using pointer semantics: * and ->
} // from here, you cannot directly access the value anymore: jolly good, since the mutex is not locked anymore!

std::cout << foo.unsafe() << std::endl; // <-- unprotected access: clearly expressed!
}

void readmeBasicUsageWithoutSafe()
{
std::mutex mutex;
std::string value;
std::lock_guard<std::mutex> lock(mutex);
value = "42";
}

void readmeBasicUsageWithSafe()
{
safe::Lockable<std::string> value;
safe::WriteAccess<safe::Lockable<std::string>> valueAccess(value);
{
safe::ReadAccess<safe::Lockable<std::string>> valueAccess(value);
}
*valueAccess = "42";
}

safe::Lockable<int, std::mutex> _1();
safe::Lockable<int> _2(); // equivalent to the above, as the second template parameter defaults to std::mutex
safe::Lockable<int&, std::mutex> _3();
safe::Lockable<int, std::mutex&> _4();
safe::Lockable<int&, std::mutex&> _5();

void readmeDefaultConstructLockableTag()
{
std::mutex aMutex;

safe::Lockable<int, std::mutex> bothDefault; // lockable and value are default constructed
safe::Lockable<int, std::mutex&> noDefault(aMutex, 42); // lockable and value are initialized
safe::Lockable<int, std::mutex&> valueDefault(aMutex); // lockable is initialized, and value is default constructed
safe::Lockable<int, std::mutex> lockableDefaultTag(safe::default_construct_mutex, 42); // lockable is default constructed, and value is initialized
safe::Lockable<int, std::mutex> lockableDefaultBraces({}, 42);
}

void readmeFlexiblyConstructLock()
{
safe::Lockable<int> value; // given a safe object
value.mutex().lock(); // with the mutex already locked...
// Because the mutex is already locked, you need to pass the std::adopt_lock tag to std::lock_guard when you construct your Access object.

// Fortunately, all arguments passed to the Safe::writeAccess() function are forwarded to the lock constructor.
safe::WriteAccess<safe::Lockable<int>> valueAccess(value, std::adopt_lock);
}

void readmeLegacy()
{
std::mutex mutex;
int value;

// Wrap the existing variables
safe::Lockable<int&, std::mutex&> lockableValue(mutex, value);
// do not use mutex and unsafeValue directly from here on!
}

void readmeConditionVariable()
{
std::condition_variable cv;
safe::Lockable<int> value;

safe::WriteAccess<safe::Lockable<int>, std::unique_lock> valueAccess(value);
cv.wait(valueAccess.lock);
}

TEST(ReadmeSafe, Basic)
{
safe::Safe<std::vector<std::string>> vec;
vec.assign(2, "bar"); // assign new vector
auto copy = vec.copy(); // get a copy
vec.writeAccess()->front() = "foo"; // replace front only
assert(vec.readAccess()->size() == 2); // check size
}

TEST(ReadmeSafe, SharedPtrNoCopy)
{
safe::Safe<std::shared_ptr<std::vector<std::string>>> vec(2, "bar"); // the std::shared_ptr is managed internally
{
	auto view = vec.copy(); // no copy, view is a std::shared_ptr<const std::vector<std::string>>, notice the const!
	assert(view->front() == "bar");
} // view destroyed
(*vec.writeAccess())->front() = "foo";
assert((*vec.readAccess())->front() == "foo");
}

TEST(ReadmeSafe, SharedPtrCopy)
{
safe::Safe<std::shared_ptr<std::vector<std::string>>> vec(2, "bar");
auto view = vec.copy(); // again, no copy here
assert(view->front() == "bar");
(*vec.writeAccess())->front() = "foo"; // the copy happens here! the content of vec is copied into a brand new std::shared_vector, then the first element is modified
assert(view->front() == "bar"); // this is still true!
assert((*vec.readAccess())->front() == "foo"); // see ? vec does hold a difference instance than view
}