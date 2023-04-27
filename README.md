# *Every variable protected by a mutex should be wrapped with safe.*
[![build](https://github.com/LouisCharlesC/safe/actions/workflows/all_platforms.yml/badge.svg)](https://github.com/LouisCharlesC/safe/actions/workflows/all_platforms.yml)
## Contents
*safe* is a header-only library that makes code with mutexes safer and easier to understand.  
This readme will walk you through the important features of the library using several code examples. Read on, and enjoy safe mutexes!
- [Overview](#Overview)
- [Motivation](#Motivation)
- [Installation](#Installation)
- [Basic usage](#Basic-usage)
- [Main features](#Main-features)
- [Advanced usage](#Advanced-usage)
## Overview
Two class templates are at the core of *safe*: Safe and Access. Safe objects pack a mutex and a value object together. Access objects act as a lock (e.g. std::lock_guard) for the mutex and provide pointer-like access to the value object. You will learn more about Safe and Access in [Basic usage](#Basic-usage).

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
	foo = "Hello, World!"; // I access foo here, but I could very well access bar, yet barMutex is not locked!
}

std::cout << bar << std::endl; // unprotected access, is this intended ?
std::cout << baz << std::endl; // what about this access ?
```
### Mutex code with *safe*
```c++
using SafeString = safe::Safe<std::string>; // type aliases will save you a lot of typing
SafeString safeFoo; // std::string and mutex packaged together!
SafeString safeBar;
std::string baz; // now you can see that this variable has no mutex

{
	safe::WriteAccess<SafeString> foo(safeFoo); // this locks the mutex and gives you access to foo
	*foo = "Hello, World!"; // access the value using pointer dereference: * and ->
}

std::cout << safeBar.unsafe() << std::endl; // unprotected access: clearly expressed!
std::cout << baz << std::endl; // all good (remember, baz has no mutex!)
```
## Motivation
Since C++11, the standard library provides mutexes, like std::mutex, along with tools to facilitate their usage, like std::lock_guard and std::unique_lock. These are sufficient to write safe multithreaded code, but it is all too easy to write code you think is safe but actually is not. Typical mistakes are: locking the wrong mutex and accessing the value object before locking (or after unlocking) the mutex. Other minor mistakes like unnecessary locking or keeping a mutex locked for too long can also be avoided.  

*safe* prevents common mutex usage mistakes by providing tools that complement the C++ standard library. Using *safe*, you will find it much easier to protect a variable using a mutex, and your code will be easier to understand.
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
## Basic usage
The *safe* library defines the Safe and Access class templates. They are meant to replace the mutexes and locks in your code. *safe* does not offer much more functionality than mutexes and locks do, they simply make their usage safer.  
Here is the simplest way to replace mutexes and locks by Safe objects.
### Vocabulary
* *safe*: the library.
* mutex: a mutex like std::mutex.
* value object: whatever needs to be protected by the mutex.
* Safe object: combines a value object and a mutex.
* lock: an object that manages a mutex using RAII like std::lock_guard and std::unique_lock.
* Access object: a lock object that also gives pointer-like access to a value object.
* access mode: Access objects can be created with read-write or read-only behavior. Read-only Access objects are especially useful to enforce the read-only nature of C++14's std::shared_lock and boost::shared_lock_guard.
### Include the library's single header
```c++
#include <safe/safe.h>
```
### Replace your values and mutexes by Safe objects
```c++
// std::mutex mutex;
// int value;
safe::Safe<int> safeValue;
```
### Replace your lock objects by Access objects
Access objects can either be read-write or read-only. The examples below show different ways to create a WriteAccess object, Replace Write/write by Read/read to get ReadAccess objects.
```c++
// std::lock_guard<std::mutex> lock(mutex); // forgot about locks
safe::WriteAccess<safe::Safe<int>> value(safeValue);
safe::Safe<int>::WriteAccess<> value(safeValue); // equivalent to the above
auto value = safeValue.writeAccess(); // nicer, but only with C++17 and later
```
#### The problem with std::lock_guard
The last line of the above example only compiles with C++17 and later. This is because of the new rules on temporaries introduced in C++17, and because *safe* uses std::lock_guard by default. std::lock_guard is non-copiable, non-moveable so it cannot be initialized as above prior to C++17. As shown below, using std::unique_lock (which is moveable) is fine:
```c++
auto value = safeValue.writeAccess<std::unique_lock>(); // ok even pre-C++17, using a std::unique_lock rather than a std::lock_guard
```
### Access your value object through the Access objects using pointer dereference
You can now safely access the value object *through the Access object*. As long as the Access object is alive, you have access the value object, and the mutex is guaranteed to be locked!
```c++
// value = 42;
*value = 42;
```
#### Use Safe member functions as one-liners, if suitable
If you need to peform a single access to your value, you can do this using Safe's member functions: `readAccess()`, `writeAccess()`, `copy()` and `assign()`. `readAccess()` and `writeAccess()` will return an Access object, but will let you operate on it in an expressive way. Example:
```c++
*safeValue.writeAccess() = 42;
int value = *safeValue.readAccess();
int value = *safeValue.writeAccess(); // this also works...
// *safeValue.readAccess() = 42; // but this obviously doesn't!
```
However, if all you need to do is assign a new value, then you might as well use the `assign()` function:
```c++
safeValue.assign(42);
```
And if you just want a copy, you can call the `copy()` function:
```c++
int value = safeValue.copy();
```
***Warning: avoid multiple calls to these functions, as each will lock and unlock the mutex.***
*Be aware that copy/move construction/assignment operators are deleted for Safe objects. That is because copying and moving requires the mutex to be locked, and the safe library aims at making every locking explicit.* Use the copy() and assign() functions instead.
## Main features
### Safety and clarity
No more locking the wrong mutex, no more mistaken access outside the safety of a locked mutex. No more naked shared variables, no more plain mutexes lying around and no more *mutable* keyword (ever locked a member mutex variable within a const-qualified member function ?).
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
See [this section](#With-legacy-code) for an example of using reference types to deal with legacy code.
#### Flexibly construct the value object and mutex
Just remember: the first argument to a Safe constructor is used to construct the mutex, the other arguments are used for the value object.  
*Note: when constructing a Safe object and the mutex is default constructed but the value object is not, you must pass the safe::default_construct_mutex tag or a set of curly brackets {} as the first constructor argument.*  
Examples:
```c++
std::mutex aMutex;

safe::Safe<int, std::mutex> bothDefault; // mutex and value are default constructed
safe::Safe<int, std::mutex&> noDefault(aMutex, 42); // mutex and value are initialized
safe::Safe<int, std::mutex&> valueDefault(aMutex); // mutex is initialized, and value is default constructed
safe::Safe<int, std::mutex> mutexDefaultTag(safe::default_construct_mutex, 42); // mutex is default constructed, and value is initialized
safe::Safe<int, std::mutex> mutexDefaultBraces({}, 42);
```
#### Flexibly construct the Lock objects
The Access constructors have a variadic parameter pack that is forwarded to the Lock object's constructor. This can be used to pass in standard lock tags such as std::adopt_lock, but also to construct your custom locks that may require additionnal arguments than just the mutex.
```c++
safe::Safe<int> safeValue; // given a Safe object
safeValue.mutex().lock(); // with the mutex already locked...
// Because the mutex is already locked, you need to pass the std::adopt_lock tag to std::lock_guard when you construct your Access object.

// No matter how you get your Access objects, you can pass arguments to the lock's constructor.
safe::WriteAccess<safe::Safe<int>> value(safeValue, std::adopt_lock);
safe::Safe<int>::WriteAccess<> value(safeValue, std::adopt_lock);
auto value = safeValue.writeAccess(std::adopt_lock); // again, only in C++17
auto value = safeValue.writeAccess<std::unique_lock>(std::adopt_lock);
```
### Even more safety!
#### Choose the access mode that suits each access
You will instatiate one Safe object for every value object you want to protect. But, you will create an Access object every time you want to operate on the value object. For each of these accesses, you can choose whether the access is read-write or read-only.
#### Force read-only access with shared mutexes and shared_locks
Shared mutexes and shared locks allow multiple reading threads to access the value object simultaneously. Unfortunately, using only mutexes and locks, the read-only restriction is not guaranteed to be applied. That is, it is possible to lock a mutex in shared mode and write to the shared value. With *safe*, you can enforce read-only access when using shared locking by using ReadAccess objects. See [this section](#Enforcing-read-only-access) for details.
### Compatibility
#### With legacy code
You can use *safe* with old-style unsafe code that uses the soon-to-be out-of-fashion separate-mutex-and-value-idiom. Imagine you are provided with the typical mutex and int. *safe* allows you to wrap these variables, without having to modify the existing code. Enjoy the safety and avoid the headaches:
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
```
## Advanced usage
### Enforcing read-only access
You can inform the *safe* library that some locks that you use are read-only (e.g. std::shared_lock, boost::shared_lock_guard). If you do so, trying to instantiate a WriteAccess object with these locks will trigger a compilation error. Use the trait class safe::AccessTraits to customize this behavior.

Here is how the trait works:
- If no specialization of the type trait exists for a lock type, the lock can be used with read-write and read-only Access objects.
- If a specialization exists, it must declare the IsReadOnly boolean variable.
  - If IsReadOnly is true, the lock is read-only: constructinfg an Access object with this lock using Mode = AccessMode::ReadWrite will fail to compile.
  - If IsReadOnly is false, the result is the same as if no specialization exists.

As an example, here is how to specialize the trait for std::shared_lock (you will find this exact code snippet in safe/accessmode.h):
```c++
template<typename MutexType>
struct AccessTraits<std::shared_lock<MutexType>>
{
	static constexpr bool IsReadOnly = true;
};
```
# Acknowledgment
Thanks to all contributors, issue raisers and stargazers!
Most cmake code comes from this repo: https://github.com/bsamseth/cpp-project and Craig Scott's CppCon 2019 talk: Deep CMake for Library Authors. Many thanks to the authors!
