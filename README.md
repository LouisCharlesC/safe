# safe
## Overview
safe is a tiny library that helps you get you multi-threaded code right and understandable. It defines the Safe class, a wrapper around a value object and the lockable object (typically a std::mutex) that protects the value for multi-threaded access. When you use safe, you clearly express:
* that the stored value needs to be protected, and which lockable object protects it.
* whether you access the value in a protected way (locking the mutex) or not.

Here is why you want to use safe:
### Without safe
```c++
std::mutex frontEndMutex;
std::mutex backEndMutex;
std::vector<int> indexesToProcess; // <-- do I need to lock a mutex to safely access this variable ?
{
	std::lock_guard<std::mutex> lock(frontEndMutex); // <-- is this the right mutex ?
	indexesToProcess.push_back(42);
}
indexesToProcess.pop_back(); // <-- unprotected access, is this intended ?
```
### With safe
```c++
// Convenience typedef
using SafeVectorInt = safe::Safe<std::vector<int>&, std::mutex&>;

std::mutex frontEndMutex;
std::mutex backEndMutex;
std::vector<int> indexesToProcess;
SafeVectorInt safeIndexes(frontEndMutex, indexesToProcess); // <-- value-mutex association!
{
	safe::StdLockGuardAccess<SafeVectorInt> indexesToProcess(safeIndexes); // <-- right mutex: guaranteed!
	indexesToProcess->push_back(42); // access the vector using pointer semantics: * and ->
}
safeIndexes.unsafe().pop_back(); // <-- unprotected access: clearly expressed!
```
## Vocabulary
* safe: the safe library
* Value object: whatever needs to be protected by a lockable object.
* Lockable object: an object that exhibits the Lockable interface: lock, try_lock and unlock.
* Safe class: wraps a value object and a lockable object.
* Lock object: an object that manages a lockable object. Examples are std::lock_guard and std::unique_lock.
* Access class: manages a lockable object and gives access to the value object.
* Locking behavior: the combination of the lockable object and the lock object define the locking behavior. I see two axes of locking behavior: lock_guard vs unique_lock, and shared vs exclusive access. To achieve shared locking, you need both a shared lockable (e.g. c++17's std::shared_mutex) and a shared lock (e.g. c++14's shared_unique_lock and boost's shared_lock_guard). The SharedAccess alias complements these classes by only providing const access to the value.
## Main features:
### 1. Store the value object/lockable object inside the Safe object, or refer to existing objects
You can use any combination of reference and non-reference types for your Safe objects:
```c++
safe::Safe<int, std::mutex>;
safe::Safe<int>; // equivalent to the above, as the second template parameter defaults to std::mutex
safe::Safe<int&, std::mutex>;
safe::Safe<int, std::mutex&>;
safe::Safe<int&, std::mutex&>;
```
### 2. Flexibly construct the Value and Lock objects.
*Note: when the lockable object is default constructed, but the value object is not, you must pass the safe::default_construct_lockable tag.* Example:
```c++
std::mutex aMutex;

safe::Safe<int, std::mutex> bothDefault; // value and lockable are default constructed, ok
safe::Safe<int, std::mutex&> noDefault(aMutex, 42); // value and lockable initialized, ok
safe::Safe<int, std::mutex&> valueDefault(aMutex); // value is default constructed, and lockable is initialized, ok
safe::Safe<int, std::mutex> lockableDefault(safe::default_construct_lockable, 42); // value is initialized to 42, and mutex is default constructed: need the safe::default_construct_lockable tag!
```
### 3. Choose the locking behavior that suits each access.
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
Through the Access class you specify the Lock object you want to use, access the Value object using pointer semantics (* and ->) and define whether you want this access to be const or read-write. For every access into a Safe object,
* define the locking behavior by choosing the LockType template parameter.
* decide whether the access should be const-qualified (SharedAccess alias) or not (Access alias).
* access the value using pointer semantics: * and ->.
### The helper aliases
The helper aliases are the way to declare Access objects suitable for a Safe object. The Safe class defines Access and SharedAccess which are templated on the Lock type. Four other aliases exist in the safe namespace for c++11's locks:
* StdLockGuardSharedAccess: std::lock_guard Lock with const access to the Value object.
* StdUniqueLockSharedAccess: std::unique_lock Lock with const access to the Value object.
* StdLockGuardAccess: std::lock_guard Lock with read-write access to the Value object.
* StdUniqueLockAccess: std::unique_lock Lock with read-write access to the Value object.

Add the aliases you need within the safe namespace for a uniform way to define your Access objects!
## Interesting use cases
### std::unique_lock to std::lock_guard
You can start by defining an Access parameterized with std::unique_lock, and later transfer the lockable's ownership to a std::lock_guard Access. The syntax is a little tricky, so here it is:
```c++
safe::Safe<int> safeValue;

safe::StdUniqueLockAccess<safe::Safe<int>> uniqueLockAccess(safeValue);
safe::StdLockGuardAccess<safe::Safe<int>> lockGuardAccess(*uniqueLockAccess, *uniqueLockAccess.lock.release(), std::adopt_lock);
```
### Access objects and std::condition_variable
You can use Access objects with std::condition_variable if you specified a Lock object that can be locked and unlocked at will, like std::unique_lock:
```c++
std::condition_variable cv;
safe::Safe<int> safeValue;

safe::StdUniqueLockAccess<safe::Safe<int>> access(safeValue);
cv.wait(access.lock);
```
### Returning a StdLockGuardAccess object
Most of the time when you use safe, you will have a Safe object as a private member variable of your class and use it to access its value object in a safe way. Example:
```c++
#include "safe.hpp"

class SafeInt
{
public:
	void set(int value)
 	{
  		*safe::StdLockGuardAccess(m_safeValue) = value;
	}

private:
	safe::Safe<int> m_safeValue;
};
```
When a client calls the set() function, you lock the Safe object using a std::lock_guard, and then change its value. This is done as a one liner: constructing the StdLockGuardAccess object, dereferencing to get a reference to the int, assigning it the new value and destructing the StdLockGuardAccess object. This is all nice and good, but imagine you would like the client to do more than just assign a new value to you object ? Using safe, the right way to do this is to return a StdLockGuardAccess object to the client:
```c++
...
	safe::StdLockGuardAccess<safe::Safe<int>> get()
 	{
  		return {m_safeValue};
	}
...
```
There are 2 tricks you must apply for this to work out. First, you must return the value by list-initialization. Second, the client must capture the return value by rvalue reference:
```c++
SafeInt safeInt;
auto&& intAccess = safeInt.get();
```
Through the intAccess variable, the client can access the int any way he wants. These accesses are guaranteed to be protected through the use of a std::lock_guard within intAccess. When intAccess goes out of scope, the lockable object is unlocked and the value object is not accessible anymore! Using a std::lock_guard as the LockType makes sure the client cannot carry the StdLockGuardAccess around and keep it alive inappropriately long because std::lock_guard is not copyable/moveable.

*Note: If I am not mistaken, the two tricks are not required in c++17, where you can return and capture as you normally would.*
## Complete example
The State and Resource classes from my mess repository use safe.
