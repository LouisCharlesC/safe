# safe
## Overview
Safe is a wrapper around a value object and the lockable object (typically a std::mutex) that protects the value for multi-threaded access. When you use Safe, you clearly express:
* that the stored value needs to be protected, and which lockable object protects it,
* whether you access the value in a protected way (locking the mutex) or not: the Access class gives protected access, the unsafe() function gives unprotected access.

Here is why you want to use Safe:
### Without Safe
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
### With Safe
```c++
// Convenience typedef
using SafeVectorInt = safe::Safe<std::vector<int>, std::mutex&>;

std::mutex wrong_mutex;
std::mutex right_mutex;
SafeVectorInt safeVec(right_mutex); // <-- value-mutex association!
{
  safe::StdLockGuardAccess<SafeVectorInt> vec(safeVec); // <-- right mutex: guaranteed!
  vec->push_back(42); // access the vector using pointer semantics: * and ->
}
safeVec.unsafe().pop_back(); // <-- unprotected access: clearly expressed!
```

## Vocabulary
* Value object: the object that needs to be protected by a lockable object.
* Lockable object: an object that exhibits the Lockable interface: lock, try_lock and unlock.
* Lock object: an object that manages a Lockable object. Examples are std::lock_guard and std::unique_lock.
* Locking behavior: the combination of the lockable object and the lock object. I see two axes of locking behavior: lock_guard vs unique_lock, and shared vs exclusive access. To achieve shared locking, you need both a shared lockable (e.g. c++17's std::shared_mutex) and a shared lock (e.g. c++14's shared_unique_lock and boost's shared_lock_guard). In that case, it is preferable to use a SharedAccess object, because it exhibits a const interface suitable for shared access.

## Main features:
### 1. Store the value object/lockable object inside the Safe object, or refer to existing objects
You can use any combination of reference and non-reference types for your Safe objects:
```c++
safe::Safe<int, std::mutex> // or safe::Safe<int>, the second template parameter defaults to std::mutex
safe::Safe<int&, std::mutex>
safe::Safe<int, std::mutex&>
safe::Safe<int&, std::mutex&>
```
### 2. Flexibly construct the Value and Lock objects.
*Note: when the lockable object is default constructed, but the value object is not, you must pass the safe::default_construct_lockable tag. Example:*
```c++
std::mutex aMutex;
safe::Safe<int, std::mutex> safeInt; // value and lockable are default constructed, ok
safe::Safe<int, std::mutex&> safeInt(aMutex, 42); // value and lockable initialized, ok
safe::Safe<int, std::mutex&> safeInt(aMutex); // value is default constructed, and lockable is initialized, ok
safe::Safe<int, std::mutex> safeInt(safe::default_construct_lockable, 42); // value is initialized to 42, and mutex is default constructed: need the safe::default_construct_lockable tag!
```
### 3. Choose the locking behavior that suits each access.
The Safe class defines the lockable type, and the Access class defines the lock type. This lets you choose the right locking behavior evey time you spawn an Access object from a Safe object.

## safe's interface
### The Safe class
The Safe class does a few useful things for you:
1. It hides the value object and forces you to specify how you want to access it.
2. It associates the value object with the lockable object that protects it.
3. It defines alises for the correctly templated Access classes that will manage the lockable object and give access to the value.
4. It allows you to access the value without any locking through the unsafe() functions.
### The helper aliases
The helper aliases are a uniform way to declare Access objects suitable for a Safe object. They live inside the safe namespace. There are four basic aliases, but you can add more as needed! The four aliases are:
StdLockGuardSharedAccess: std::lock_guard Lock with const access to the Value object.
StdUniqueLockSharedAccess: std::unique_lock Lock with const access to the Value object.
StdLockGuardAccess: std::lock_guard Lock with read-write access to the Value object.
StdUniqueLockAccess: std::unique_lock Lock with read-write access to the Value object.
### The Access classes
Through the Access classes you specify the Lock object you want to use and you access the Value object using pointer semantics (* and ->).

## Interesting use case
### std::unique_lock to std::lock_guard
You can start by defining an Access parameterized with std::unique_lock, and later transfer the lockable's ownership to a std::lock_guard Access. The syntax is is little tricky, so here it is:
```c++
safe::Safe<int> safeValue;
safe::StdUniqueLockAccess<safe::Safe<int>> uniqueLockAccess(safeInt);
safe::StdLockGuardAccess<safe::Safe<int>> lockGuardAccess(*uniqueLockAccess, *uniqueLockAccess.lock.release(), std::adopt_lock);
```

## Complete example
The State class from my mess repository uses safe