# Examples for the *safe* library
Here are two multi-threading utility classes that use *safe*: State and Resource. Let me explain why I coded these classes, then I will explain how they work.
## Member variables in multi-threaded code
There are a few reasons why classes in C++ have member variables. In single-threaded code, whatever the reason, you simply use the member variables you need for your class to work properly and everything is fine. In multi-threaded code, a variable's purpose influences the way you will manage the access to it. Here are a few use cases for member variables. A member variable might be:
1. an input for member functions. Example: the maximum number of iterations to run.
2. an output for member functions. Example: the maximum number of iterations to run on the next call to this member function.
3. a resource that is used over and over again by member functions (because this resource is costly to construct, for instance). Example: a large vector used to sort an input vector.
There may be other uses for member variables. In any case, if your variable does not fill one of these roles, make it a local variable to the member function that needs it and move on.
Otherwise, the member variable must be managed because concurrent calls to your member functions means concurrent accesses to the member variable which means data race (if at least one of the accesses is a write).
## States and Resources
The State class was designed to deal with input and output member variables, the Resource class deals with, well, resources.
### State
If your member variable is an input or an output of a member function, reads and writes to it must be thread-safe. That's it. The State class's public interface provides just this functionality. The **copy()** and **assign()** member functions let you read and write to the variable. *copy()* returns a local copy of the variable you can use for the duration of the function. *assign()* replaces the content of the variable.

Now imagine your variable is an std::vector and you are only interested in knowing its size. Are your going to copy the whole vector only to call size() on it ? Of course not. You will use the **readLock()** member function provided by the State class! *readock()* returns a ReadAccess object to the variable, allowing you to perform any operation you want on it without incurring the cost of a copy. Likewise, if you only want to modify the first element of the vector, it would be a shame to replace it as a whole using the *assign()* function. It is much better to use the **writeLock()** member function to get a WriteAccess object to the variable. That way, you can alter any part of the variable you like in isolation.

Thread-safe *copy()*, *assign()*, *readLock()* and *writeLock()*: this is all the State class is about.
#### Specialization for State<std::shared_ptr>
State objects of std::shared_ptr are interesting because the reference counting apparatus of the shared pointer allows for a very nice optimization: copy-on-write. For this class template specialization, calls to copy() do not make a copy of the pointed-to variable, they return a const std::shared_ptr to the variable. From there, a copy of the variable *may* happen, but only if this returned shared_ptr still exists when the next call to *assign()* or *writeLock()* happens.  
That is, if you  
call copy(), destroy the returned shared_ptr and then call assign(): no copy will ever be made.  
On the other hand, if you  
call copy() and then call assign(): a copy will be made *by the call to assign()*.  
The same holds for calls to writeLock().  
Calls to readLock() never cause copies because the State's mutex is locked while the ReadAccess object exists, delaying calls to any other member function.
### Resource
If your member variable is a resource, use the Resource class. Resources are different than States: they do not transfer information between threads, they only are meant to be available for usage. Multiple threads that need a specific resource at a given point in time should access *a different instance* of this resource.

When you construct a Resource object, you construct multiple instances of the variable. When you call the **get()** member function, you get a WriteAccess object for one of these instances of the variable. When you destroy the WriteAccess object, this instance becomes available again for other threads to use.

The Resource class effectively provides the behavior of a non-static thread_local member variable.
