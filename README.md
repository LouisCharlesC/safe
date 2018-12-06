# Safe
## Overview
Small wrapper around a value object and the lockable object (typically a std::mutex) that protects it for multi-threaded access. The value is hidden by the Safe object until safe or unsafe accessor functions are called. The unsafe() accessor functions are provided for convinience, the safe way to access the value is by getting a Safe::Guard or a Safe::Lock that manage the lockable object while providing access to the value.
* Guard and ConstGuard objects act like std::lock_guard (RAII for a lockable object, without the possibility to unlock it during the lock_guard's lifetime) and give access to the protected value.
2. Lock and ConstLock objects act like std::unique_lock (RAII for a lockable object, but with the possibility to unlock and relock at will) and give access to the protected value. You can also access the lock direcly, it is a public member variable.

## Philosophy
The primary use of Safe is to unambiguously link a lockable object and the value that it protects. Secondly, the safe accessors (Guard, ConstGuard, Lock and ConstLock) ensure that the lockalbe is locked when the value is accessed. Finally, the unsafe accessors allow unprotected access to the value when locking is superfluous, use at your own risk.
    * Safe also makes intent clearer: when you provide a Safe (that is, both a value and a lockable tied together), you explicitely tell which values are protected by a lockable object and which are not. Protected values are inside the Safe, unprotected ones are outside!
    * Safe makes it harder to forget to lock before fiddling with the value, because the value cannot be accessed directly: an accessor function must be called first.

## Example
The safe::State class uses a Safe. Please take a look a the State class, as the example below uses one.
```cpp
write the examples...
};
``` 



