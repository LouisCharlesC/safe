# safe
## Overview
safe is a tiny library that helps you get you multi-threaded code right and understandable. It defines the Safe class, a wrapper around a value object and the lockable object (typically a std::mutex) that protects the value for multi-threaded access. When you use safe, you clearly express:
* that the stored value needs to be protected, and which lockable object protects it.
* whether you access the value in a protected way (locking the mutex) or not: the Access classes give protected access, the unsafe() functions give unprotected access.

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
* Value object: whatever needs to be protected by a lockable object.
* Lockable object: an object that exhibits the Lockable interface: lock, try_lock and unlock.
* Lock object: an object that manages a lockable object. Examples are std::lock_guard and std::unique_lock.
* Locking behavior: the combination of the lockable object and the lock object. I see two axes of locking behavior: lock_guard vs unique_lock, and shared vs exclusive access. To achieve shared locking, you need both a shared lockable (e.g. c++17's std::shared_mutex) and a shared lock (e.g. c++14's shared_unique_lock and boost's shared_lock_guard). In that case, it is preferable to use a SharedAccess object, because it exhibits a const interface suitable for shared access.

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
### The Access classes
Through the Access classes you specify the Lock object you want to use, access the Value object using pointer semantics (* and ->) and define whether you want this access to be const or read-write.
### The helper aliases
The helper aliases are the way to declare Access objects suitable for a Safe object. The Safe class defines Access and SharedAccess which are templated on the Lock type. Four other aliases exist in the safe namespace for c++11's locks:
* StdLockGuardSharedAccess: std::lock_guard Lock with const access to the Value object.
* StdUniqueLockSharedAccess: std::unique_lock Lock with const access to the Value object.
* StdLockGuardAccess: std::lock_guard Lock with read-write access to the Value object.
* StdUniqueLockAccess: std::unique_lock Lock with read-write access to the Value object.
Add any other alias you need within the safe namespace for a uniform way to define you Access objects!

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
## Complete example
The State class from my mess repository uses safe.
