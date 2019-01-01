# Safe
## Overview
Small wrapper around a value object and the lockable object (typically a std::mutex) that protects the value for multi-threaded access. When you use Safe, you clearly express:
* that the stored value needs to be protected, and which lockable object protects it: they are packed together,
* whether you access the value in a protected or unprotected way: calling lock() or guard() gives protected access, calling unsafe() gives unprotected access.

Here is why you want to use Safe:
### Without Safe
```c++
std::mutex the_wrong_mutex;
std::mutex the_right_mutex;
std::vector<int> vec;
{
  // Wrong mutex, but how could you tell ?
  std::lock_guard<std::mutex> lock(the_wrong_mutex);
  vec.push_back(42);
}
// Unprotected access, is this intended ?
vec.pop_back();
```
### With Safe
```c++
std::mutex the_wrong_mutex;
// The right mutex is in the Safe object
safe::Safe<std::vector<int>> safeVec;
{
  // Right mutex: guaranteed!
  decltype(safeVec)::Guard vec(safeVec);
  vec.push_back(42);
}
// Unprotected access: clearly expressed!
safeVec.unsafe().pop_back();
```

## Main features:
### 1. Use internal or external lockable object
```c++
safe::Safe&lt;int, std::mutex&gt; safeInternal; // a std::mutex lives in safeInternal
safe::Safe&lt;int&gt; safeInternalDefault; // Lockable template parameter defaults to std::mutex
std::mutex mutex;
safe::Safe&lt;int, std::mutex&&gt; safeExternal(mutex); // a reference to mutex is stored
safe::Safe&lt;int, std::mutex&&gt; safeSameMutex(mutex); // uses the same mutex as safeExternal
```
Note: when the lockable object is default constructed, but the value object is not, you must pass the safe::default_construct_lockable tag. Example:
```c++
safe::Safe&lt;int&gt; safeInternalDefault(safe::default_construct_lockable(), 42)
```
### 2. c++11 compatible, but seamlessly integrates c++14 and c++17 std::shared_mutex and std::shared_lock.
### 3. Just like the standard library, choose between a lock_guard or a unique_lock behavior. 

## Complete example




