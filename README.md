# Safe
## Overview
Safe is a wrapper around a value object and the lockable object (typically a std::mutex) that protects the value for multi-threaded access. When you use Safe, you clearly express:
* that the stored value needs to be protected, and which lockable object protects it: they are packed together,
* whether you access the value in a protected or unprotected way: the Guard and Lock classes give protected access, the unsafe() function gives unprotected access.

Here is why you want to use Safe:
### Without Safe
```c++
std::mutex wrong_mutex;
std::mutex right_mutex;
std::vector<int> vec;
{
  std::lock_guard<std::mutex> lock(wrong_mutex); // <-- wrong mutex, but how could you tell ?
  vec.push_back(42);
}
vec.pop_back(); // <-- unprotected access, is this intended ?
```
### With Safe
```c++
// Convenience typedef
using SafeVectorInt = safe::Safe<std::vector<int>, std::mutex&>;

std::mutex wrong_mutex;
std::mutex right_mutex;
SafeVectorInt safeVec(right_mutex); // <-- value+mutex association!
{
  safe::StdLockGuardAccess<SafeVectorInt> vec(safeVec); // <-- right mutex: guaranteed!
  vec->push_back(42); // access the vector using pointer semantics: * and ->
}
safeVec.unsafe().pop_back(); // <-- unprotected access: clearly expressed!
```

## Main features:
### 1. Just like the standard library, choose between a lock_guard or a unique_lock behavior. 
### 2. c++11 compatible *and* std::shared_lock (c++14) and std::shared_mutex (c++17) ready.
Introduce safe::NonShared...
### 3. Use internal or external lockable object
```c++
safe::Safe<int, safe::NonShared<std::mutex>>; safeInternal; // a std::mutex lives in safeInternal
safe::Safe<int> safeInternalDefault; // Lockable template parameter defaults to safe::NonShared<std::mutex>
std::mutex myMutex;
safe::Safe<int, safe::NonShared<std::mutex&>> safeExternal(myMutex); // a reference to myMutex is stored
safe::Safe<int, safe::NonShared<std::mutex&>> safeSameMutex(myMutex); // uses the same mutex as safeExternal
```
Note: when the lockable object is default constructed, but the value object is not, you must pass the safe::default_construct_lockable tag. Example:
```c++
safe::Safe<int> safeInternalDefault(safe::default_construct_lockable(), 42)
```

## Complete example




