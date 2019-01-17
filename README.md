# Make your multi-thread code *safe* and crystal clear!
## Overview
safe is a tiny library that helps you get your multi-threaded code right and understandable. It defines the Safe and Access classes. A Safe object combines a lockable object (e.g. std::mutex) and a value object (whatever you need to protect with the lockable object). You cannot directly access the value through the Safe object.
C++11 introducing RAII for mutexes with the std::lock_guard and std::unique_lock classes. In the RAII idiom, the state of the lockable object is tied to the lifetime of a lock object. Safe pushes this of step further with Access objects by also tying the possibility to access the value object to the lock's lifetime.

Here is why you want to use safe:
### Without safe
```c++
std::mutex frontEndMutex;
std::mutex backEndMutex;
int count; // <-- do I need to lock a mutex to safely access this variable ?
{
	std::lock_guard<std::mutex> lock(frontEndMutex); // <-- is this the right mutex ?
	++count;
}
--count; // <-- unprotected access, is this intended ?
```
### With safe
```c++
std::mutex frontEndMutex;
safe::Safe<int> safeCount; // <-- value and mutex packaged together!
{
	safe::LockGuard<safe::Safe<int>> count(safeCount); // <-- right mutex: guaranteed!
	++*count; // access the vector using pointer semantics: * and ->
}
--safeCount.unsafe(); // <-- unprotected access: clearly expressed!
```
## Vocabulary
* safe: the safe library.
* Value object: whatever needs to be protected by a lockable object.
* Lockable object: an object that exhibits the Lockable interface: lock, try_lock and unlock.
* Safe object: Hides the value object until you lock the lockable object.
* Lock object: an object that manages a lockable object. Examples are std::lock_guard and std::unique_lock.
* Access object: locks the lockable object and gives access to the value object.
* Locking behavior: the combination of the lockable object and the lock object define the locking behavior. In safe, there are two axes of locking behavior: lock_guard vs unique_lock, and shared vs exclusive access. To achieve shared locking, you need both a shared lockable (e.g. c++17's std::shared_mutex) and a shared lock (e.g. c++14's shared_unique_lock and boost's shared_lock_guard). The read-only Access class complements these by only providing const access to the value.
## Main features:
### 1. Choose any lockable and lock classes that fit your needs!
The Safe class is templated on the lockable object: use std::mutex, std::shared_mutex (c++17), or you own tubo lock-free mutex!

The Access class is templated on the lock object: use std::lock_guard, boost::shared_lock_guard, or you own low-entropy double-turn lock!
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
### 4. Choose the locking behavior that suits each access.
The Safe class defines the lockable type, and the Access class defines the lock type. This lets you choose the right locking behavior every time you spawn an Access object from a Safe object.
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
* decide whether the access should be const-qualified (SharedAccess alias) or not (Access alias).
* access the value using pointer semantics: * and ->.
### The helper aliases
The helper aliases are the way to declare Access objects suitable for a Safe object. The Safe class defines Access and SharedAccess which are templated on the Lock type. Four other aliases exist in the safe namespace for c++11's locks:
* **LockGuard** access using std::lock_guard: locking is tied to the lifetime of the access object.
* **UniqueLock** access using std::unique_lock: you can unlock and re-lock the lockable at will, suitable to use with std::condition_variable.

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
### Returning a LockGuard object
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
When a client calls the increment() function, you lock the Safe object using a std::lock_guard, and then change its value. Here, this is done as a one liner: constructing the LockGuard object (locking the mutex), dereferencing it to get a reference to the int, incrementing the int and destructing the LockGuard object (unlocking the mutex). This is all nice and good, but imagine you would like the client to do more than just increment the count ? Will you write a function for every operation the client might want to use ? Even then, the locking and unlocking will be too fine-grained, as it will happen at every call of a function. The right thing to do is to return a LockGuard object to the client:
```c++
...
	safe::LockGuard<safe::Safe<int>> get()
 	{
  		return {m_safeCount};
	}
...
```
There are 2 tricks you must apply for this to work out. First, you must return the value by list-initialization. Second, the client must capture the return value by rvalue reference:
```c++
MultithreadCount multithreadNbrOfLinesOfCode;
{
	auto&& nbrOfLinesOfCode = multithreadNbrOfLinesOfCode.get();
	--nbrOfLinesOfCode;
}
```
Through the nbrOfLinesOfCode variable, the client can do whatever he likes with the count value. For the lifetime of nbrOfLinesOfCode, the mutex will be locked thanks to the use of std::lock_guard. When nbrOfLinesOfCode goes out of scope, the mutex is unlocked and the count is not accessible anymore. For this usage, do use a safe::LockGuard, because these cannot be transfered from one scope to another, and thus clients are less likely to keep the nbrOfLinesOfCode variable alive for too long (keep in mind: the mutex is locked as long as nbrOfLinesOfCode lives, so it better be short-lived!).

*Note: If I am not mistaken, the two tricks are not required in c++17. If you use c++17 you can return and capture as you normally would.*

## Complete example
The State and Resource classes from my mess repository use safe.
