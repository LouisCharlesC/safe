# Safe
## Overview
Small wrapper around a value object and the lockable object (typically a std::mutex) that protects the value for multi-threaded access. When you use Safe, you:
* clearly express that the stored value needs to be protected, and which lockable object protects it.
* clearly know whether you access the value in a protected or unprotected way. Calling lock() or guard() gives you protected access, calling unsafe() gives unprotected access.

Here is why you want to use Safe:
<table border="0">
 <tr>
    <td><b style="font-size:30px">Without Safe</b></td>
    <td><b style="font-size:30px">With Safe</b></td>
 </tr>
 <tr>
    <td><pre><code class="language-c++">
std::mutex the_wrong_mutex;
std::mutex the_right_mutex;
std::shared_ptr&lt;int&gt; ptr;
{
  // Wrong mutex, but how could you tell ?
  std::lock_guard&lt;std::mutex&gt; lock(the_wrong_mutex);
  *ptr = 1;
}
// Unprotected access, is this intended ?
std::cout << ptr.unique() << std::endl;
    </code></pre></td>
    <td><pre><code class="language-c++">
std::mutex the_wrong_mutex;
// The right mutex is in the Safe object
safe::Safe&lt;std::shared_ptr&lt;int&gt;&gt; safePtr;
{
  // The right mutex: guaranteed!
  decltype(safePtr)::Guard ptr(safePtr);
  *ptr = 1;
}
// Unprotected access: clearly expressed!
std::cout << ptr.unsafe().unique() << std::endl;
    </code></pre></td>
 </tr>
</table>

Other features:
1. Easy integration of c++14 and c++17 std::shared_mutex and std::shared_lock.
2. Use internal or external lockable object
3. Just like std, choose between a unique lock or a lock guard

Complete example: State.



