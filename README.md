# *Every variable protected by a mutex should be wrapped with safe.*
[![build](https://github.com/LouisCharlesC/safe/actions/workflows/all_platforms.yml/badge.svg)](https://github.com/LouisCharlesC/safe/actions/workflows/all_platforms.yml)
## Contents
*safe* is a header-only library that makes code with mutexes safer and easier to understand.  
This readme will walk you through the important features of the library using several code examples. Read on, and enjoy safe mutexes!
- [Overview](#overview)
- [Motivation](#motivation)
- [Main features](#main-features)
- [Installation](#installation)
- [Advanced usage](#advanced-usage)
## Overview
Two class templates are at the core of *safe*: Safe and Access. Safe objects pack a mutex and a value object together. Access objects act as a lock (e.g. std::lock_guard) for the mutex and provide pointer-like access to the value object.

Here is why you want to use *safe*:
### Mutex code without *safe*
```c++
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
```
### Mutex code with *safe*
```c++
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
```
## Motivation
Since C++11, the standard library provides mutexes, like std::mutex, along with tools to facilitate their usage, like std::lock_guard and std::unique_lock. These are sufficient to write safe multithreaded code, but it is all too easy to write code you think is safe but actually is not. Typical mistakes are: locking the wrong mutex and accessing the value object before locking (or after unlocking) the mutex. Other minor mistakes like unnecessary locking or keeping a mutex locked for too long can also be avoided.  

*safe* prevents common mutex usage mistakes by providing tools that complement the C++ standard library. Using *safe*, you will find it much easier to protect a variable using a mutex, and your code will be easier to understand. No more locking the wrong mutex, no more mistaken access outside the safety of a locked mutex. No more naked shared variables, no more plain mutexes lying around and no more *mutable* keyword (ever locked a member mutex variable within a const-qualified member function ?).
## Main features
### Flexibility
#### Choose the mutex and lock that fit your need
The Safe class template has a template parameter for the mutex: 
- use std::mutex, std::shared_mutex (C++17), name it!

The Access class template has a template parameter for the lock object: 
- use std::lock_guard, boost::shared_lock_guard, anything you want!
- you can use the lock you need for every Access object you construct.
#### Store the value object and mutex inside the Safe object, or refer to existing objects
You can use any combination of reference and non-reference types for your Safe objects:
```c++
safe::Safe<int, std::mutex>;
safe::Safe<int>; // equivalent to the above, as the second template parameter defaults to std::mutex
safe::Safe<int&, std::mutex>;
safe::Safe<int, std::mutex&>;
safe::Safe<int&, std::mutex&>;
```
See [this section](#with-legacy-code) for an example of using reference types to deal with legacy code.
#### Flexibly construct the value object and mutex
The Safe constructor accepts the arguments needed to construct the value and the mute object. The last argument is forwarded to the mutex constructor and the rest to the value's.  
If the last argument cannot be used to construct the mutex, *safe* detects it and forwards everything to the value constructor.  
If you explicitely do not want to use the last argument to construct the mutex object, use the safe::default_construct_mutex tag as last argument.  
Examples:
```c++
std::mutex mutex;
safe::Safe<std::pair<int,int>, std::mutex> bothDefault;            // mutex and value are default constructed
safe::Safe<std::pair<int,int>, std::mutex &> noDefault(42, 24, mutex); // mutex and value are initialized
safe::Safe<std::pair<int,int>, std::mutex &> valueDefault(mutex);  // mutex is initialized, and value is default constructed
safe::Safe<std::pair<int,int>, std::mutex> mutexDefault(42, 24); // mutex is default constructed, and value is initialized
safe::Safe<std::pair<int,int>, std::mutex> mutexDefaultTag(42, 24, safe::default_construct_mutex); // mutex is default constructed, and value is initialized
```
See 
#### Flexibly construct the Lock objects
The Access constructors have a variadic parameter pack that is forwarded to the Lock object's constructor. This can be used to pass in standard lock tags such as std::adopt_lock, but also to construct your custom locks that may require additionnal arguments than just the mutex.   
There are many equivalent ways to get an Access object from a Safe object. Choose the syntax you prefer:
```c++
safe::Safe<int> safeValue;
const safe::ReadAccess<safe::Safe<int>> value(safeValue);
const safe::Safe<int>::ReadAccess<> value(safeValue); // the empty <> are unfortunately necessary.
const auto value = safeValue.readLock(); // nicer, but only with C++17 and later
const auto value = safeValue.readLock<std::unique_lock>(); // ok even pre-C++17, using a std::unique_lock rather than the default std::lock_guard
const safe::Safe<int>::ReadAccess<std::unique_lock> value(safeValue); // equivalent to the above
// All of the above exist in read-write versions, simply replace "read" by "write" (and remove the const).
```
See [this section](#with-stdlock_guard) to understand why the nicer version doesn't work pre-C++17.

Examples passing arguments to the Lock object:
```c++
std::mutex aLockedMutex;
aLockedMutex.lock();
safe::Safe<int, std::mutex&> safeValue(aLockedMutex); // given a Safe object with an already locked mutex.

// Because the mutex is already locked, you need to pass the std::adopt_lock tag to std::lock_guard when you construct your Access object.
// No matter how you get your Access objects, you can pass arguments to the lock's constructor.
// Again, all the lines below are equivalent but here we need to specify the mutex type for the Safe type, because it is not the default (notice the &):
safe::WriteAccess<safe::Safe<int, std::mutex&>> value(safeValue, std::adopt_lock);
safe::Safe<int, std::mutex&>::WriteAccess<> value(safeValue, std::adopt_lock);
auto value = safeValue.writeLock(std::adopt_lock); // again, only in C++17
auto value = safeValue.writeLock<std::unique_lock>(std::adopt_lock); // using a std::unique_lock still works
```
```c++
// Here's a safe int using a timed_mutex
safe::Safe<int, std::timed_mutex> safeValue;

// Pass in a time-out when you lock the mutex, and it will try to lock for that duration!
// safe does not do anything here, std::unique_lock does it all. safe simply forwards any argument
// it gets, and when std::unique_lock receives a std::duration, it try to lock before giving up.
auto value = safeValue.writeLock<std::unique_lock>(std::chrono::seconds(1));
// You can reach the lock through the Access object and check whether or not the locking was successful:
assert(value.lock.owns_lock());
```
### Even more safety!
#### Choose the access mode that suits each access
You will instatiate one Safe object for every value object you want to protect. But, you will create an Access object in every scope where you want to operate on the value object. For each of these accesses, you can choose whether the access is read-write or read-only.
#### Force read-only access with shared_locks
Shared mutexes and shared locks allow multiple reading threads to access the value object simultaneously. Unfortunately, using only mutexes and locks, the read-only restriction is not guaranteed to be applied. That is, it is possible to lock a mutex in shared mode and write to the shared value. With *safe*, you can enforce read-only access when using shared locking by using ReadAccess objects. See [this section](#enforcing-read-only-access) for details.
### Compatibility
#### With legacy code
You can use *safe* with old-style unsafe code that uses the soon-to-be out-of-fashion separate-mutex-and-value idiom. Imagine you are provided with the typical mutex and int. *safe* allows you to wrap these variables, without having to modify the existing code. Enjoy the safety and avoid the headaches:
```c++
std::mutex lousyMutex;
int unsafeValue;

// Wrap the existing variables
safe::Safe<int&, std::mutex&> safeValue(lousyMutex, unsafeValue);
// do not use lousyMutex and unsafeValue directly from here on!
```
#### With code from the future
*safe* is written in C++11, but it is fully compatible with mutexes and locks from different sources like C++14's std::shared_lock and C++17's std::shared_mutex, thanks to template parameters. Of course, you can also use boost::shared_lock_guard and your own custom mutexes and locks.
#### With standard uses of mutexes and locks
The mutex is accessible from the Safe object through an accessor functions, and the lock object is a public member of the Access class. Anything you can do with your typical mutexes and locks you can do with *safe*. 

For example, *safe* can seamlessly be used with std::condition_variable:
```c++
std::condition_variable cv;
safe::Safe<int> safeValue;
safe::Safe<int>::WriteAccess<std::unique_lock> value(safeValue);
cv.wait(value.lock, [](){return true;});
// use `value` as you usually would.
```
#### With std::lock_guard
Some code shown in this readme does not compile with C++ versions prior C++17. This is because of the new rules on temporaries introduced in C++17, and because *safe* uses std::lock_guard by default. std::lock_guard is non-copiable, non-moveable so it cannot be initialized as nicely as other locks prior to C++17. If you don't like using std::lock_guard, check out the [Advanced Usage](#advanced-usage) section.
## Installation
### Method 1: Copy the source files in your project
*safe* is a header-only library. Using the library can simply mean copy the contents of the include/ folder to some place of your convenience. This is the most straightforward installation method.

### Method 2: Via CMake FetchContent (CMake > 3.14)
```CMake
cmake_minimum_required(VERSION 3.14)
project(my_project)

FetchContent_Declare(
  safe
  GIT_REPOSITORY https://github.com/LouisCharlesC/safe.git
  GIT_TAG        v1.1.0
)
FetchContent_MakeAvailable(safe)

add_executable(my_project my_project.cc)
target_link_library(my_project safe::safe)
```
NOTE: `find_package(safe CONFIG REQUIRED)` is not needed with this method.
### Method 3: Install locally via CMake
```bash
git clone https://github.com/louischarlescaron/safe

cd safe
cmake -B safe-build -DCMAKE_INSTALL_PREFIX="$(pwd)/safe-install"
cmake --build safe-build --config Release --target install
```
Then you can use `find_package` in your project:
```cmake
cmake_minimum_required(VERSION 3.11)
project(my_project)

find_package(safe CONFIG REQUIRED)

add_executable(my_project my_project.cc)
target_link_library(my_project safe::safe)
```
And build with:
```bash
cd my_project
cmake -B build -DCMAKE_PREFIX_PATH="path/to/safe-install"
cmake --build build --config Release
```
NOTE: `CMAKE_PREFIX_PATH` is used to tell `find_package()` where to look for libraries. `path/to/safe-install` is not a standard path but it's easier to remove when needed.
### Method 4: Install system-wide via CMake (not recommended)
```bash
git clone https://github.com/LouisCharlesC/safe

cd safe
cmake -B build
sudo cmake --build build --config Release --target install
```
*safe* will be installed into your OS's standard intallation path. Be aware that system-wide installation make it hard to deal with multilpe library versions, and can cause collisions if you happen to install another library called safe!

When you build your own project, you **won't** need to append `-DCMAKE_PREFIX_PATH="path/to/safe-install"`.
## Advanced usage
### Enforcing read-only access
You can inform the *safe* library that some locks that you use are read-only (e.g. std::shared_lock, boost::shared_lock_guard). If you do so, trying to instantiate a WriteAccess object with these locks will trigger a compilation error. Use the trait class safe::AccessTraits to customize this behavior.

Here is how the trait works:
- If no specialization of the type trait exists for a lock type, the lock can be used with read-write and read-only Access objects.
- If a specialization exists, it must declare the IsReadOnly boolean variable.
  - If IsReadOnly is true, the lock is read-only: constructinfg an Access object with this lock using Mode = AccessMode::ReadWrite will fail to compile.
  - If IsReadOnly is false, the result is the same as if no specialization exists.

As an example, here is how to specialize the trait for std::shared_lock (you will find this exact code snippet in safe/access_mode.h):
```c++
template<typename MutexType>
struct safe::AccessTraits<std::shared_lock<MutexType>>
{
	static constexpr bool IsReadOnly = true;
};
```
### Avoid some typing by defining your own default lock types
*safe* uses std::lock_guard by default everywhere. If you know you will always use a certain lock type given some mutex type (for instance, std::unique_lock with std::timed_mutex), you can inform *safe* and it will use this lock by default with this mutex type.  
To do so, you must specialize the safe::DefaultLocks class template. Have a look at the tests/test_default_locks.cpp files. As you can see, you can specify a different lock type for read and write accesses:
```c++
template<>
struct safe::impl::DefaultLocks<std::timed_mutex>
{
    using ReadOnly = std::unique_lock<std::timed_mutex>;
    using ReadWrite = std::lock_guard<std::timed_mutex>;
};
```
# Acknowledgment
Thanks to all contributors, issue raisers and stargazers!
The cmake is inspired from https://github.com/bsamseth/cpp-project and Craig Scott's CppCon 2019 talk: Deep CMake for Library Authors. Many thanks to the authors!
