# Safe
## Overview
Small wrapper around a value object and the lockable object (typically a std::mutex) that protects it for multi-threaded access. The Safe class hides the value until one of two locking functions is called:
1. Safe::lock_guard() yields a SafeGuard object which acts exactly like std::lock_guard (RAII for a lockable object, without the possibility to unlock it during the lock_guard's lifetime) and also is a handle to the protected value. You access the value with operator*() and operator->().
2. Safe::unique_lock() yields a SafeLock object which acts exactly like std::unique_lock (RAII for a lockable object, but with the possibility to unlock and relock at will) and also is a handle to the protected value. You access the value with operator*() and operator->(). You access the lock direcly, it is a public member of SafeLock.

## Philosophy
1. The primary use of Safe is to carry both a value that must be protected with the lockable object that protects it. The lockable object is typically unlocked until the value needs to be accessed. This way,
    * Safe makes it harder to forget to lock before fiddling with the value, because the value cannot be accessed directly: a locking function must be called first.
    * Safe also makes intent clearer: when you provide a Safe (that is, both a value and a lockable tied together), you explicitely tell which values are protected by a lockable object and which are not. Protected values are inside the Safe, unprotected ones are outside!
2. It can also be useful to provide an already locked Safe (either a SafeGuard or a SafeLock) if a value needs to be protected and you want to allow a client to act on it, but you do not know exactly what the client might want to do. If you provide the client with a SafeGuard, you effectively provide unconstrained but protected access to the value. Of course, it is the responsability of the client to destroy the SafeGuard as soon as possible so that the lockable gets unlocked.

## Example
```cpp
template<typename ValueType>
class MultiThreadedValue
{
public:
	template<typename... Args>
	MultiThreadedValue(Args&&... args):
		m_safeValue(m_mutex, std::forward<Args>(args)...)
	{}

	// Call get() if you need to store the equivalent of a reference to the int, because you might want to operate on it later
	const safe::Safe<int>& get() const
	{
		return m_safeValue;
	}

	// This sets m_safeValue to a new value, as expected
	void set(const ValueType& value)
	{
		*m_safeValue.lock_guard() = value; // you need to lock_guard() the Safe in order to access the value with operator*()
	}

	// If you want to modify m_safeValue, but in a more complicated way, call update() and act on the Safe<ValueType>::Guard object
	typename safe::Safe<ValueType>::Guard update()
	{
		return m_safeValue.lock_guard();
	}

private:
	std::mutex m_mutex;
	safe::Safe<ValueType> m_safeValue;
};
``` 



