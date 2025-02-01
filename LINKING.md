# Understanding C++ Compilation Process

## Introduction to Header Files and Source Files

### Basic Structure
In C++, code is typically organized into two types of files:
- Header files (`.h`, `.hpp`) containing declarations
- Source files (`.cpp`, `.cc`, `.cxx`) containing implementations

### Preprocessor Directives
Preprocessor commands always start with `#` and are processed before actual compilation:
```cpp
#include <iostream>     // Include system header
#include "myheader.h"  // Include local header
#define MAX_SIZE 100   // Macro definition
#ifdef DEBUG          // Conditional compilation
```

### Function Organization Best Practices

In header file (`math.h`):
```cpp
#ifndef MATH_H
#define MATH_H

class Calculator {
public:
    double add(double a, double b);  // Declaration only
};

#endif
```

In source file (`math.cpp`):
```cpp
#include "math.h"

double Calculator::add(double a, double b) {  // Definition
    return a + b;
}
```

### Header-Only vs Split Implementation
Sometimes functions are defined entirely in headers:
```cpp
// header-only.hpp
template<typename T>
T max(T a, T b) {
    return (a > b) ? a : b;
}
```

**Why not define everything in headers?**
1. Compilation time impact
   - Each source file including the header needs recompilation when the header changes
   - For large projects, this can significantly slow down builds
2. Binary size implications
   - Risk of code bloat due to inline expansion
3. Encapsulation benefits
   - Implementation details hidden from users of the header

## Declaration vs Definition

### Variables
```cpp
// Declaration
extern int counter;  // Declares counter without defining it
extern const double PI;  // Declares constant PI

// Definition
int counter = 0;  // Defines and initializes counter
const double PI = 3.14159;  // Defines constant PI
```

### Functions
```cpp
// Declaration (in header)
void processData(const std::vector<int>& data);

// Definition (in source)
void processData(const std::vector<int>& data) {
    // Implementation here
}
```

### Classes
```cpp
// Declaration
class Widget;  // Forward declaration

// Definition
class Widget {
    int value;
public:
    Widget(int v) : value(v) {}
};
```

## Compilation Process in Detail

```
Source Files (.cpp)     Header Files (.h)
       ↓                      ↓
    Preprocessor
       ↓
Translation Units (.i)
       ↓
    Compiler
       ↓
Object Files (.o/.obj)
       ↓
    Linker
       ↓
  Executable (.exe)
```

### 1. Preprocessing Stage
- Processes all `#` directives
- Expands macros
- Removes comments
- Includes header files

### 2. Compilation Stage
- Generates object files (.o or .obj)
- Contains machine code
- Includes symbol tables
- Contains metadata for linking

### 3. Linking Stage
- Resolves external references
- Combines object files
- Links with libraries
- Produces final executable

### Common File Extensions
- `.cpp`, `.cc`, `.cxx`: C++ source files
- `.h`, `.hpp`: Header files
- `.o`, `.obj`: Object files
- `.a`, `.lib`: Static libraries
- `.so`, `.dll`: Dynamic libraries
- `.i`: Preprocessed source
- `.exe`, `.out`: Executables

### Template Generation

Templates are handled through a process called instantiation:

```cpp
template<typename T>
T add(T a, T b) {
    return a + b;
}

// Usage creates these instantiations:
int result1 = add(5, 3);        // Creates add<int>
double result2 = add(3.14, 2.0); // Creates add<double>
```

The compiler:
1. Identifies needed template instantiations
2. Generates specific code for each type
3. Includes generated code in object files
4. Uses the One Definition Rule (ODR) to ensure consistency

### One Definition Rule (ODR)
- Each entity can have only one definition in the entire program
- Templates are an exception (must be identical in all translation units)
- Inline functions can be defined multiple times (must be identical)

Example of ODR violation:
```cpp
// file1.cpp
inline int getValue() { return 42; }

// file2.cpp
inline int getValue() { return 43; } // ODR violation!
```

## Common Issues and Debug Tips
1. Undefined reference errors
   - Missing implementation
   - Forgot to link object file
2. Multiple definition errors
   - ODR violation
   - Missing include guards
3. Template instantiation errors
   - Definition not visible at point of use
   - Type requirements not met

## Resoures
- https://cliutils.gitlab.io/modern-cmake/chapters/basics/structure.html
- "Linkers and Loaders" by John R. Levine
