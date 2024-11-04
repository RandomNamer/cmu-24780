# Learned from the homeworks

## PS3
### [ps3-2](ps3/ps3-2.cpp)

#### In C++, how do we write a function to return a list?

- `vector<> func()`: Return by value, resulting in move (creates a vector in caller stack, but the new vector points to the same content in heap) or SVO (simply create one object on caller stack frame).
- `const vector<> func()`: Uses copy constructor since it's marked as const, which the heap space in vector is also copied.
- `vector<> &func()` or `const vector<> &func()`: returns the reference, no copy or move, but it's dangerous if the vector is allocated inside the function's stack frame.
- Pass a vector reference of the caller as parameter.

In the end, it's better to use `vector<> func()` and let the caller decide whether it's const.

#### Others
- `GL_LINES`: Draw lines between each pair of vertices.
- `switch-case` by default does not introduce scope across cases, which may cause jump bypasses variable initialization error if we want to declare references in one case. We can manually introduce a scope by using `{}`.

### [ps9](ps9/)

#### And how do we write a function to return an array?

