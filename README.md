# Make your multi-thread code safe and crystal clear!
## Overview
*safe* is a header-only library that helps you get your multi-threaded code safe and understandable. It defines the Safe and Access classes. A Safe object packs a lockable object (e.g. std::mutex) and a value object (whatever you need to protect using the lockable object). Safe objects expose the value object through a simple, clear and safe interface: use the access() member functions to gain protected access to the value, and use the unsafe() functions for unprotected access. Protected access is achieved through the Access class. Think of the Access class as a combination of a lock (e.g. std::lock_guard) and a pointer to the value object. The lock gives you the full power of RAII for managing the lockable object, and the pointer-like functionality only exists for the span of time where the lockable object is locked.

Here is why you want to use safe:
### Without safe
```c++
std::mutex frontEndMutex;
std::mutex backEndMutex;
int value; // <-- do I need to lock a mutex to safely access this value ?
{
	std::lock_guard<std::mutex> lock(frontEndMutex); // <-- is this the right mutex ?
	++value;
}
--value; // <-- unprotected access, is this intended ?
```
### With safe
```c++
std::mutex frontEndMutex;
safe::Safe<int> safeValue; // <-- value and mutex packaged together!
{
	safe::Safe<int>::Access<std::lock_guard> value(safeValue); // <-- right mutex: guaranteed!
	++*value; // access the value using pointer semantics: * and ->
} // from here, you cannot directly access the value anymore: jolly good, since the mutex is not locked anymore!
--safeValue.unsafe(); // <-- unprotected access: clearly expressed!
```
## Vocabulary
* *safe*: the safe library.
* Value object: whatever needs to be protected by a lockable object.
* Lockable object: an object that exhibits the BasicLockable interface: lock() and unlock(). Examples are std::mutex and std::recursive_mutex.
* Safe object: combines a value object and a lockable object. Expose the value object through a simple and expressive interface.
* Lock object: an object that manages a lockable object. Examples are std::lock_guard and std::unique_lock.
* Access object: a Lock object that also gives pointer-like access to the value object.
* Access mode: Access objects can be created with read-write or read-only behavior. Read-only Access objects are especially useful to enforce the read-only nature of std::shared_mutex (c++17) and std::shared_lock (c++14).
## Main features:
### 1. Safety and clarity
#### Get your code right
No more naked shared variables, no more locking the wrong mutex, no more mistaken access outside the safety of a locked mutex.
#### Hide those ugly details
No more plain mutexes lying around, no more locks, no more mutable (ever used a member mutex variable within a const-qualified member function ?).
### 2. Choose any lockable and lock that fit your needs
The Safe class is templated on the lockable object: use std::mutex, std::shared_mutex (c++17), name it! 
The Access class is templated on the lock object: use std::lock_guard, boost::shared_lock_guard, anything you want!
### 3. Store the value object/lockable object inside the Safe object, or refer to existing objects
You can use any combination of reference and non-reference types for your Safe objects:
```c++
safe::Safe<int, std::mutex>;
safe::Safe<int>; // equivalent to the above, as the second template parameter defaults to std::mutex
safe::Safe<int&, std::mutex>;
safe::Safe<int, std::mutex&>;
safe::Safe<int&, std::mutex&>;
```
### 4. Flexibly construct the Value and Lock objects
*Note: when the lockable object is default constructed, but the value object is not, you must pass the safe::default_construct_lockable tag. Example:*
```c++
std::mutex aMutex;

safe::Safe<int, std::mutex> bothDefault; // lockable and value are default constructed
safe::Safe<int, std::mutex&> noDefault(aMutex, 42); // lockable and value are initialized
safe::Safe<int, std::mutex&> valueDefault(aMutex); // lockable is initialized, and value is default constructed
safe::Safe<int, std::mutex> lockableDefault(safe::default_construct_lockable, 42); // lockable is default constructed, and value is initialized
```
### 5. Choose the lock and access mode that suits each access
One you construct a Safe object, you fix the type of the lockable object you will use. From there, you will create an Access object every time you want to operate on the value object. For each of these accesses, you can choose the appropriate lock, and whether the access is read-write or read-only.
#### Force read-only access with shared mutexes and shared_locks
Shared mutex and shared locks allow multiple reading threads to access the value object simultaneously. Unfortunately, using only mutexes and locks, the read-only restriction is not guaranteed to be applied. That is, it is easy to create a situation where a thread locks a mutex in shared mode and writes to the shared value. With safe, you can enforce read-only access when using shared locking.
## Basic usage
### Include the header-only library
```c++
#include "safe.hpp"
```
### Replace your value object by a Safe object...
and get rid of the mutexes in the process.
```c++
// without safe
std::mutex mutex;
int value;
// with safe
safe::Safe<int> safeValue;
```
```c++
// using c++17's std::shared_mutex
// without safe
std::shared_mutex mutex;
int value;
// with safe
safe::Safe<int, std::shared_mutex> safeValue;
```
### Replace your lock objects by Access objects
```c++
std::lock_guard<std::mutex> lock(mutex); // without safe
safe::Safe<int>::Access<std::lock_guard> value(safeValue); // with safe
```
```c++
std::unique_lock<std::mutex> lock(mutex); // without safe
safe::Safe<int>::Access<std::unique_lock> value(safeValue); // with safe
```
#### Using Access<std::unique_lock> with std::condition_variable
```c++
std::condition_variable cv;
safe::Safe<int> safeValue;
safe::Safe<int>::Access<std::unique_lock> value(safeValue);
cv.wait(value.lock);
```
### Access your value object though Access objects using pointer semantics
```c++
value = 42; // without safe
*value = 42; // with safe
```
## Going a little bit deeper
### One-liners
The plain Safe::access() member function returns an Access<std::lock_guard> object with read-write behavior. This allows you to write safe and compact one-liners.
```c++
safe::Safe<std::vector<int>> safeVector;
*safeVector.access() = std::vector(1, 2);
safeVector.access()->clear();
```
#### Capturing the return of Safe::access()
The Access<std::lock_guard> object returned by a call to Safe::access() is non-copyable, non-moveable (because it aggregates a non-copyable, non-moveable std::lock_guard object). Thus, it cannot be captured with a typicall syntax, an rvalue reference must be used:
```c++
// auto value = safeValue.access(); // <-- does not compile!
auto&& value = safeValue.access(); // notice the rvalue reference: auto&&
```
This is totally safe, altough  slightly annoying. Note that with c++17 and later, it is no longer necessary to use the rvalue reference (if I am not mistaken!).
### Specifying the access mode
The Access class has one template parameter for the lock type, and another one for the access mode. Likewise, templated overloads of the Safe::access() member function exist with a parameter for the lock type and one for the access mode. The AccessMode template parameter is an enum which can take two possible values: ReadOnly and ReadWrite. Here are examples of read-only Access objects:
```c++
safe::Safe<int>::Access<std::lock_guard, AccessMode::ReadOnly> value(safeValue);
``````c++
auto&& value = safeValue.access<std::lock_guard, AccessMode::ReadOnly>();
```
#### Specifying a default access mode for a lock type
Here is the full declaration of the Access class:
```c++
template<template<typename> class LockType, AccessMode ReadOrWrite=LockTraits<LockType>::DefaultAccessMode> class Access
```
The AccessMode template parameter defines the access mode for the Access class. The AccessMode template parameter has a default value, and this default value is defined by a type trait of the LockType type. If no specialization of the type trait exists for LockType, the default access mode is ReadWrite. If a specialization exists, it must declare the DefaultAccessMode variable which determines the default access mode for the lock type. It is useful to specify a ReadOnly default access mode for shared locks like std::shared_unique_lock. Example:
```c++
template<>
struct LockTraits<std::shared_unique_lock>
{
	static constexpr AccessMode DefaultAccessMode = AccessMode::ReadOnly;
};
```
## Advanced uase cases
### Templated safe code
Writing templated code using safe reveals a little bit too much the true nature of c++ because the Safe class contains template nested types and functions. It means that if your Safe object is templated, you need to use the following convoluted syntax (**nptice the "::template " and ".template " syntax**):
```c++
template <typename Valuetype>
class Example
{
public:
	safe::Safe<ValueType> m_safeValue;

	void exampleType()
	{
		safe::Safe<ValueType>::template Access<std::lock_guard> value(m_safeValue);
	}
	void exampleMemberFunction()
	{
		auto&& value = m_safeValue.template access<std::lock_guard>();
	}
};
```
### Returning an Access object
Most of the time when you use safe, you will have a Safe object as a private member variable of your class and use it to access its value object in a safe way. Example:
```c++
#include "safe.hpp"

class MultithreadCount
{
public:
    void increment()
     {
          ++*m_safeCount.access();
    }

private:
    safe::Safe<int> m_safeCount;
};
```
When a client calls the increment() function, you lock the Safe object's mutex using an Access<std::lock_guard> object, and then increment the value. This is all nice and good, but imagine you would like the client to do more than just increment the count ? Will you write a function for every operation the client might want to use ? Even if you do, the locking and unlocking will be too fine-grained, as it will happen at every call of a function. The right thing to do is to return an Access<std::lock_guard> object to the client:
```c++
    safe::Safe<int>::Access<std::lock_guard> get()
     {
          return {m_safeCount};
    }
```
Notice the list-initialization of the returned Access<std::lock_guard> object. Pre-c++17, it is mandatory to use this syntax, again because Access<std::lock_guard> is non-copyable, non-moveable.
## Complete examples
The State and Resource classes from my *mess* repository use *safe*.
