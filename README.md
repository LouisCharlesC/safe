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
std::vector<int> vec;
std::mutex the_right_mutex;
std::mutex the_wrong_mutex;
{
  std::lock_guard<std::mutex> lock(the_wrong_mutex); // <-- wrong mutex, but how could you tell ?
  vec.resize(1);
  vec.push_back(1);
}
vec.pop_back(); // <-- unprotected access, is this intended ?
    </code></pre></td>
    <td><pre><code class="language-c++">
safe::Safe<std::vector<int>> safeVec;
std::mutex the_wrong_mutex;
{
  auto vec = safeVec.guard(); // <-- right mutex!!
  vec->resize(1);
  vec->push_back(1);
}
safeVec.unsafe().pop_back(); // <-- unprotected access clearly expressed!!
    </code></pre></td>
 </tr>
</table>
