# Make your multi-thread code *safe* and crystal clear!
## Overview
*safe* is a header-only library that helps you get your multi-threaded code safe and understandable. It defines the Safe and Access classes. A Safe object packs a lockable object (e.g. std::mutex) and a value object (whatever you need to protect using the lockable object). Safe objects expose the value object through a simple, clear and safe interface: use the access() member functions to gain protected access to the value, and use the unsafe() functions for unprotected access. Protected access is achieved through the Access class. Think of the Access class as a combination of a lock (e.g. std::lock_guard) and a pointer to the value object. The lock gives you the full power of RAII for managing the lockable object, and the pointer-like functionality only exists for the span of time where the lockable object is locked.

Here is why you want to use safe:
### Without safe
```c++
std::mutex frontEndMutex;
std::mutex backEndMutex;
int value; // <-- do I need to lock a mutex to safely access this variable ?
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
	safe::LockGuard<safe::Safe<int>> value(safeValue); // <-- right mutex: guaranteed!
	++*value; // access the value using pointer semantics: * and ->
} // from here, you cannot directly access the value anymore: jolly good, since the mutex is not locked anymore!
--safeValue.unsafe(); // <-- unprotected access: clearly expressed!
```
## Vocabulary
* safe: the safe library.
* Value object: whatever needs to be protected by a lockable object.
* Lockable object: an object that exhibits the BasicLockable interface: lock() and unlock(). Examples are std::mutex and std::recursive_mutex.
* Safe object: Combines a value object and a lockable object. Expose the value object through a simple and expressive interface.
* Lock object: an object that manages a lockable object. Examples are std::lock_guard and std::unique_lock.
* Access object: a Lock object that also gives pointer-like access to the value object.
* Access mode: read-write or read-only. Access objects can be created with read-write or read-only behavior. Read-only Access objects are especially useful to enforce the read-only nature of std::shared_mutex*es* (c++17) and std::shared_lock*s* (c++14).
## Main features:
### 1. Safety and clarity
#### Get your code right
No more naked shared variables, no more locking the wrong mutex, no more mistaken access outside the safety of a locked mutex.
#### Hide those ugly details
No more plain mutexes lying around, no more locks, no more mutable (ever used a member mutex varible within a const-qualified member function ?).
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
### Replace your value objects by Safe objects
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
The type aliases in the safe namespace are meant to replace your daily std::lock_guards and std::unique_locks. Examples:
```c++
std::lock_guard<std::mutex> lock(mutex); // without safe
safe::LockGuard<safe::Safe<int>> value(safeValue); // with safe
```
```c++
std::unique_lock<std::mutex> lock(mutex); // without safe
safe::UniqueLock<safe::Safe<int>> value(safeValue); // with safe
```
```c++
// using c++14's std::shared_unique_lock
std::shared_unique_lock<std::mutex> lock(mutex); // without safe
safe::SharedUniqueLock<safe::Safe<int, std::shared_mutex>> value(safeValue); // with safe
```
### Access your value objects though the Access objects using pointer semantics
```c++
value = 42; // without safe
*value = 42; // with safe
```
## Going a little bit deeper
### One-liners
The Safe::access() member function yields a safe::LockGuard object with read-write behavior. This allows you to write safe and compact one-liners.
```c++
safe::Safe<std::vector<int>> safeVector;
*safeVector.access() = std::vector(1, 2);
safeVector.access()->clear();
```
#### One-liners with other locks

There are 3 ways you can create Access objects:
1. the Safe::access() function and its overloads. These functions are useful to write one-liners.
2. the Access type aliases within the safe namsspace. They provide a syntax similar to locks in the standard library, they also simplify writing templated code with safe.
3. the Safe::Access class. The class allows you to use safe with any lock class you want.
### Safe::access member function
There are three overloads to this function to provide a easy to use and customizable interface.
#### Safe::access()
The plain Safe::access() function yields a Safe::Access<std::lock_guard, safe::ReadWrite> object. That is, an Access object using a std::lock_guard and with read-write access to the value object.
#### Safe::access\<typename LockType\>()
#### Safe::access\<typename LockType, AccessMode ReadOrWrite\>()

## Basic usage

### Non-one-liners
If you need to perform several operations on your value object, please do not use the one-liners! Either use the type aliases as in the *Replace locks by accesses* example, or store the return value of the call to Safe::access(). Here are the two different ways to rework one-liner example:
```c++
safe::Safe<std::vector<int>> safeVector;
{
	auto&& vector = safeVector.access();
	*vector = std::vector(1, 2);
	vector->clear();
}
```
```c++
safe::Safe<std::vector<int>> safeVector;
{
	safe::LockGuard<safe::Safe<std::vector<int>>> vector(safeVector);
	*vector = std::vector(1, 2);
	vector->clear();
}
```
**Notice the rvalue-reference** when capturing the return value of the call to Safe::access(). It is **mandatory** to use this syntax, else your compiler will complain in a very difficult to understand way. The rvalue-reference is necessary because the safe::LockGuard object returned by Safe::accesss() has a std::lock_guard member variable. std::lock_guard is non-copyable, non-moveable, hence safe::LockGuard also is non-copyable, non-moveable. This means it is only possible to capture a safe::LockGuard return value with an rvalue-reference. Honestly, it is no big deal, just type && and it will work fine.  
In c++17, you do not need this rvalue reference, you can capture non-copyalbe, non-moveable return values as you normally would.
### Access objects and std::condition_variable
You can use Access objects with std::condition_variable if you specified a Lock object that can be locked and unlocked at will, like std::unique_lock:
```c++
std::condition_variable cv;
safe::Safe<int> safeValue;

auto value = safeValue.access<std::unique_lock>();
cv.wait(value.lock);
```
## Advanced use cases
### std::unique_lock to std::lock_guard
You can start by defining an Access parameterized with std::unique_lock, and later transfer the lockable's ownership to a std::lock_guard Access. The syntax is a little tricky, so here it is:
```c++
safe::Safe<int> safeValue;

safe::UniqueLock<safe::Safe<int>> uniqueLockAccess(safeValue);
safe::LockGuard<safe::Safe<int>> lockGuardAccess(*uniqueLockAccess, *uniqueLockAccess.lock.release(), std::adopt_lock);
```
### Templated safe code
Writing templated code using safe reveals a little bit too much the true nature of c++ because the Safe class contains template nested types and functions. It means that if your Safe object is templated, you need to use the following convoluted syntax:
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
The type aliases defined in the safe namespace are there to partially alleviate this problem, as they allow you to use the standard syntax even within templated code. As a matter of fact, the following codes compiles and works as expected:
```c++
	void exampleAlias()
	{
		safe::LockGuard<Safe<ValueType>> value(m_safeValue);
	}
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
          ++*safe::LockGuard<safe::Safe<int>>(m_safeCount);
    }

private:
    safe::Safe<int> m_safeCount;
};
```
When a client calls the increment() function, you lock the Safe object using a std::lock_guard, and then change its value. Here, this is done as a one liner: constructing the LockGuard object (locking the mutex), dereferencing it to get a reference to the int, incrementing the int and destructing the LockGuard object (unlocking the mutex). This is all nice and good, but imagine you would like the client to do more than just increment the count ? Will you write a function for every operation the client might want to use ? Even then, the locking and unlocking will be too fine-grained, as it will happen at every call of a function. The right thing to do is to return a Access object to the client:
```c++
...
    safe::LockGuard<safe::Safe<int>> get()
     {
          return {m_safeCount};
    }
...
```
Returning a safe::UniqueLock is straightforward, but if you want the Access object to be a safe::LockGuard, there are 2 tricks you must apply. First, you must return the value by list-initialization, as shown above. Second, the client must capture the return value by rvalue reference:
```c++
MultithreadCount count;
{
    auto&& countAccess = multithreadCount.get();
    --nbrOfLinesOfCode;
}
```
Through the countAccess variable, the client can do whatever he likes with the count value. For the lifetime of countAccess, the mutex will be locked thanks to the use of std::lock_guard. When nbrOfLinesOfCode goes out of scope, the mutex is unlocked and the count is not accessible anymore. I like safe::LockGuard objects for this usage, because they cannot be transfered from one scope to another, and thus clients are less likely to keep them alive for too long (keep in mind: the mutex is locked as long as countAccess lives, so it better be short-lived!).

*Note: If I am not mistaken, these two tricks are not required in c++17. If you use c++17 you can return and capture a non-copyable-moveable variable as you normally would a copyable/movealbe one.*

## Complete examples
The State and Resource classes from my *mess* repository use *safe*.
