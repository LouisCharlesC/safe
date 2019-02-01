# Make your multi-thread code *safe* and crystal clear!
## Overview
safe is a header-only library that helps you get your multi-threaded code right and understandable. It defines the Safe and Access classes. A Safe object packs a lockable object (e.g. std::mutex) and a value object (whatever you need to protect using the lockable object). Safe objects expose the value object through a simple, clear *and safe* interface: use the access() member functions to gain protected access to the value, and use the unsafe() functions for unprotected access. Protected access is achieved through the Access class. Think of the Access class as a combination of a lock (e.g. std::lock_guard) and a pointer to the value object. The lock gives you the full power of RAII for managing the lockable object, and the pointer-like functionality only exists for the span of time where the lockable object is locked.

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
	safe::LockGuard<safe::Safe<int>> value(safeNbrOfWillyWallers); // <-- right mutex: guaranteed!
	++*value; // access the value using pointer semantics: * and ->
}
--safeValue.unsafe(); // <-- unprotected access: clearly expressed!
```
## Vocabulary
* safe: the safe library.
* Value object: whatever needs to be protected by a lockable object.
* Lockable object: an object that exhibits the BasicLockable interface: lock() and unlock(). Examples are std::mutex and std::recursive_mutex.
* Safe object: Combines a value object and a lockable object. Expose the value object through a simple interface.
* Lock object: an object that manages a lockable object. Examples are std::lock_guard and std::unique_lock.
* Access object: a Lock object that also gives access to the value object.
* Access mode: read-write or read-only. Access objects can be created with read-write or read-only behavior. Read-only Access objects are especially useful to enforce the read-only nature of shared_mutex (c++17) and shared_lock (c++14).
## Main features:
### 1. Choose any lockable and lock that fit your needs!
The Safe class is templated on the lockable object: use std::mutex, std::shared_mutex (c++17), name it!
The Access class is templated on the lock object: use std::lock_guard, boost::shared_lock_guard, anything you want!
### 2. Store the value object/lockable object inside the Safe object, or refer to existing objects
You can use any combination of reference and non-reference types for your Safe objects:
```c++
safe::Safe<int, std::mutex>;
safe::Safe<int>; // equivalent to the above, as the second template parameter defaults to std::mutex
safe::Safe<int&, std::mutex>;
safe::Safe<int, std::mutex&>;
safe::Safe<int&, std::mutex&>;
```
### 3. Flexibly construct the Value and Lock objects.
*Note: when the lockable object is default constructed, but the value object is not, you must pass the safe::default_construct_lockable tag.* Example:
```c++
std::mutex aMutex;

safe::Safe<int, std::mutex> bothDefault; // value and lockable are default constructed, ok
safe::Safe<int, std::mutex&> noDefault(aMutex, 42); // value and lockable initialized, ok
safe::Safe<int, std::mutex&> valueDefault(aMutex); // value is default constructed, and lockable is initialized, ok
safe::Safe<int, std::mutex> lockableDefault(safe::default_construct_lockable, 42); // value is initialized to 42, and mutex is default constructed: need the safe::default_construct_lockable tag!
```
### 4. Choose the lock and access mode that suits each access.
One you construct a Safe object, you fix the type of the lockable object you will use. From there, you will create an Access object every time you want to access your value. For each if these accesses, you can choose the appropriate lock, and whether the access is read-write or read-only.
## Safe's interface
### The Safe class
The Safe class does a few useful things for you, it:
1. hides the value object.
2. associates the value object with the lockable object that protects it.
3. forces you to specify how you want to access the value object.
3. defines alises for the correctly templated Access classes that will manage the lockable object and give access to the value.
4. allows you to access the value without any locking through the unsafe() functions.
### The Access class
Through the Access class you specify the Lock object you want to use, access the Value object using pointer semantics (* and ->) and define whether you want this access to be read-write or read-only. For every access into a Safe object,
* define the locking behavior by choosing the LockType template parameter.
* decide whether the access should is read-write or read only.
* access the value using pointer semantics: * and ->.
### The helper aliases
The helper aliases are the way to declare Access objects suitable for a Safe object. The Safe class defines Access and SharedAccess which are templated on the Lock type. Four other aliases exist in the safe namespace for c++11's locks:
* **LockGuard** access using std::lock_guard: locking is tied to the lifetime of the access object.
* **UniqueLock** access using std::unique_lock: you can unlock and re-lock the lockable at will, suitable to use with std::condition_variable.
* **SharedLock** access using std::shared_lock (c++14): multiple threads can access the value object in read-only mode simultaneously.

Add the aliases you need within the safe namespace for a uniform way to define your Access objects!
## Interesting use cases
### std::unique_lock to std::lock_guard
You can start by defining an Access parameterized with std::unique_lock, and later transfer the lockable's ownership to a std::lock_guard Access. The syntax is a little tricky, so here it is:
```c++
safe::Safe<int> safeValue;

safe::UniqueLock<safe::Safe<int>> uniqueLockAccess(safeValue);
safe::LockGuard<safe::Safe<int>> lockGuardAccess(*uniqueLockAccess, *uniqueLockAccess.lock.release(), std::adopt_lock);
```
### Access objects and std::condition_variable
You can use Access objects with std::condition_variable if you specified a Lock object that can be locked and unlocked at will, like std::unique_lock:
```c++
std::condition_variable cv;
safe::Safe<int> safeValue;

safe::UniqueLock<safe::Safe<int>> access(safeValue);
cv.wait(access.lock);
```
### Returning a Access object
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

*Note: If I am not mistaken, the two tricks are not required in c++17. If you use c++17 you can return and capture a non-copyable-moveable variable as you normally would a copyable/movealbe one.*

## Complete example
The State and Resource classes from my mess repository use safe.
