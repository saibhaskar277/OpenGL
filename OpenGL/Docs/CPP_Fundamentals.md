# C++ Fundamentals — Complete Theory Guide

Native **C++** for general programming: data types, memory, pointers, OOP, abstraction, and optimization — with clear use cases and examples.

Graphics/OpenGL + GLSL live in a separate doc: [OpenGL_Engine_Guide.md](OpenGL_Engine_Guide.md)

---

## Table of contents

1. [How a C++ program runs](#1-how-a-c-program-runs)
2. [Data types](#2-data-types)
3. [Variables, scope, and lifetime](#3-variables-scope-and-lifetime)
4. [Operators and control flow](#4-operators-and-control-flow)
5. [Functions](#5-functions)
6. [Pointers](#6-pointers)
7. [References](#7-references)
8. [Arrays and C-strings](#8-arrays-and-c-strings)
9. [Dynamic memory](#9-dynamic-memory)
10. [RAII and smart pointers](#10-raii-and-smart-pointers)
11. [Structs and classes (OOP)](#11-structs-and-classes-oop)
12. [Encapsulation, abstraction, inheritance, polymorphism](#12-encapsulation-abstraction-inheritance-polymorphism)
13. [Constructors, destructors, copy, move](#13-constructors-destructors-copy-move)
14. [const, static, constexpr, namespaces](#14-const-static-constexpr-namespaces)
15. [Templates and the STL](#15-templates-and-the-stl)
16. [Error handling](#16-error-handling)
17. [Optimization tips](#17-optimization-tips)
18. [Modern C++ habits (C++17)](#18-modern-c-habits-c17)
19. [Practice exercises](#19-practice-exercises)
20. [Quick reference](#20-quick-reference)

---

## 1. How a C++ program runs

```text
Source (.cpp/.h) → Compiler → Object files → Linker → Executable
                                      ↓
                                 Run: main()
```

```cpp
#include <iostream>

int main() {
    std::cout << "Hello\n";
    return 0; // 0 = success
}
```

| Piece | Role |
|-------|------|
| Header (`.h`) | Declarations shared between files |
| Source (`.cpp`) | Definitions / implementations |
| `#include` | Textual inclusion of a header |
| `main` | Program entry point |

**Use case:** every console or GUI app starts in `main` (or a framework that eventually calls your code).

---

## 2. Data types

### Fundamental types

| Type | Typical size* | Range idea | Use case |
|------|---------------|------------|----------|
| `bool` | 1 byte | true/false | Flags |
| `char` | 1 | Character / small integer | Text, bytes |
| `int` | 4 | Whole numbers | Counters, indices |
| `unsigned int` | 4 | ≥ 0 | Sizes, IDs |
| `long long` | 8 | Big integers | Timestamps, big counts |
| `float` | 4 | ~7 decimal digits | Real numbers (fast) |
| `double` | 8 | ~15 decimal digits | Higher precision math |
| `void` | — | “No type” | Functions that return nothing |

\*Sizes are platform-dependent; use `sizeof(T)` when it matters.

```cpp
int lives = 3;
float speed = 2.5f;      // f suffix = float literal
double pi = 3.1415926535;
bool alive = lives > 0;
char grade = 'A';
```

### `sizeof`

```cpp
std::cout << sizeof(int) << '\n';    // often 4
std::cout << sizeof(double) << '\n'; // often 8
```

**Use case:** knowing byte sizes when packing data, serializing, or allocating buffers.

### `auto`

```cpp
auto x = 10;        // int
auto y = 3.14;      // double
auto z = 3.14f;     // float
```

**Use case:** avoid repeating long types; still clear from the initializer.

### Typedefs / aliases

```cpp
using Health = int;
using Callback = void (*)(int);

Health hp = 100;
```

**Use case:** give domain meaning to plain types; shorten ugly names.

---

## 3. Variables, scope, and lifetime

### Scope

```cpp
int global = 1; // global / namespace scope — live for whole program

void f() {
    int local = 2; // automatic (stack) — dies when f returns
    {
        int inner = 3; // dies at end of this block
    }
    // inner not visible here
}
```

### Lifetime summary

| Storage | Where | Lifetime |
|---------|-------|----------|
| Automatic (stack) | Locals | End of scope |
| Static / global | Data segment | Whole program |
| Dynamic (heap) | `new` / allocators | Until you free |

**Use case:** prefer the smallest scope that works — fewer bugs, clearer ownership.

---

## 4. Operators and control flow

### Arithmetic / comparison / logic

```cpp
int a = 7, b = 3;
a + b; a - b; a * b; a / b; a % b;  // 10,4,21,2,1
a == b; a != b; a < b;
(a > 0) && (b > 0);
(a > 0) || (b > 0);
!alive;
```

### Branching

```cpp
if (score >= 90) {
    grade = 'A';
} else if (score >= 80) {
    grade = 'B';
} else {
    grade = 'C';
}

switch (grade) {
    case 'A': /* ... */ break;
    case 'B': /* ... */ break;
    default:  /* ... */ break;
}
```

### Loops

```cpp
for (int i = 0; i < 10; ++i) { /* ... */ }

while (alive) { /* ... */ }

do { /* ... */ } while (condition);
```

**Use case:** `for` when you know the count; `while` when you wait on a condition (e.g. game loop until quit).

### Range-for

```cpp
std::vector<int> nums = {1, 2, 3};
for (int n : nums) {
    std::cout << n << ' ';
}
for (int& n : nums) { // modify elements
    n *= 2;
}
```

---

## 5. Functions

### Basics

```cpp
int add(int a, int b) {
    return a + b;
}

void logMessage(const std::string& msg) {
    std::cout << msg << '\n';
}
```

| Style | Meaning | Use case |
|-------|---------|----------|
| Pass by value `T` | Copy | Small types (`int`, `float`) |
| Pass by reference `T&` | Can modify caller | Out-params, big mutable objects |
| Pass by const ref `const T&` | Read without copy | Strings, vectors, structs |
| Return by value | Give a result | Most functions |

```cpp
void scale(float& value, float factor) {
    value *= factor;
}

float x = 10.0f;
scale(x, 2.0f); // x == 20
```

### Overloading

```cpp
float area(float r) { return 3.14159f * r * r; }
float area(float w, float h) { return w * h; }
```

**Use case:** same idea, different argument lists.

### Default arguments

```cpp
void teleport(float x, float y, float z = 0.0f);
```

### `[[nodiscard]]`

```cpp
[[nodiscard]] bool loadConfig(const std::string& path);
// caller warned if return value ignored
```

---

## 6. Pointers

### What they are

A pointer stores a **memory address**.

```cpp
int value = 42;
int* ptr = &value;  // address of value

std::cout << *ptr;  // 42  (dereference)
*ptr = 100;         // value is now 100
```

| Syntax | Meaning |
|--------|---------|
| `&x` | Address of `x` |
| `T*` | Pointer to `T` |
| `*p` | Object pointed to |
| `p->m` | Member `m` via pointer |
| `nullptr` | Empty pointer |

### Null checks

```cpp
int* p = nullptr;
if (p != nullptr) {
    *p = 5;
}
```

**Use case:** optional objects, C APIs, dynamic structures.

### Pointers to objects

```cpp
struct Player {
    int health;
};

Player player{100};
Player* p = &player;
p->health -= 10;
(*p).health -= 10; // same
```

### Pointer arithmetic

```cpp
int arr[3] = {10, 20, 30};
int* p = arr;
*(p + 1); // 20 — advances by sizeof(int)
```

**Use case:** walking contiguous arrays; low-level buffers.

### Dangling pointer (bug)

```cpp
int* dangerous() {
    int x = 5;
    return &x; // NEVER — x is destroyed on return
}
```

### `void*`

```cpp
void wipe(void* memory, std::size_t bytes) {
    // generic bytes; cast back when you know the type
    auto* bytesPtr = static_cast<unsigned char*>(memory);
    for (std::size_t i = 0; i < bytes; ++i) {
        bytesPtr[i] = 0;
    }
}
```

**Use case:** generic allocators, C library APIs.

### Function pointers

```cpp
int apply(int x, int (*op)(int)) {
    return op(x);
}

int square(int v) { return v * v; }

apply(4, square); // 16
```

**Use case:** callbacks, plugin-style behavior (or prefer `std::function` / lambdas in modern C++).

---

## 7. References

References are **aliases** — must bind to an existing object; cannot be null or reseated.

```cpp
int a = 10;
int& ref = a;
ref = 20; // a is 20
```

| | Pointer | Reference |
|--|---------|-----------|
| Null? | Yes | No |
| Reseat? | Yes | No |
| Syntax | `*` / `->` | `.` |

```cpp
void heal(int& health, int amount) {
    health += amount;
}
```

**Use case:** operator overloading, clean APIs, avoid copies of large objects with `const T&`.

---

## 8. Arrays and C-strings

### C-style arrays

```cpp
int scores[5] = {1, 2, 3, 4, 5};
scores[0] = 10;

// decays to pointer:
void printAll(const int* data, std::size_t count) {
    for (std::size_t i = 0; i < count; ++i) {
        std::cout << data[i] << ' ';
    }
}
printAll(scores, 5);
```

**Pitfall:** after decay, the function does **not** know the length — pass it explicitly.

### `std::array` (fixed, safe)

```cpp
#include <array>
std::array<int, 5> scores = {1, 2, 3, 4, 5};
scores.size(); // 5
```

**Use case:** fixed-size collections with bounds-aware interface.

### Multidimensional

```cpp
int grid[2][3] = {
    {1, 2, 3},
    {4, 5, 6}
};
```

### C-strings

```cpp
const char* hello = "Hello"; // null-terminated
std::string modern = "Hello"; // prefer this
modern.c_str(); // const char* view for C APIs
```

**Pitfall:**

```cpp
const char* bad = std::string("temp").c_str(); // dangling — temporary died
```

---

## 9. Dynamic memory

### `new` / `delete`

```cpp
int* p = new int(42);
delete p;
p = nullptr;

int* arr = new int[100];
delete[] arr; // must match new[]
```

### Problems with raw new/delete

- Forgetting `delete` → **leak**
- Double `delete` → **crash / UB**
- Exception between new and delete → **leak**
- Mixing `delete` and `delete[]` → **UB**

**Use case today:** rarely write raw `new` in application code; prefer containers and smart pointers.

### Heap vs stack example

```cpp
void f() {
    int stackObj = 1;                 // automatic
    std::vector<int> heapData(1000);  // vector object on stack;
                                      // 1000 ints on heap, freed in destructor
}
```

---

## 10. RAII and smart pointers

### RAII

**Resource Acquisition Is Initialization:** acquire in constructor, release in destructor.

```cpp
class FileHandle {
public:
    explicit FileHandle(const char* path) : m_file(std::fopen(path, "r")) {
        if (!m_file) throw std::runtime_error("open failed");
    }
    ~FileHandle() {
        if (m_file) std::fclose(m_file);
    }
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;
private:
    std::FILE* m_file;
};
```

**Use case:** files, sockets, locks, GPU handles — anything that must be released.

### `std::unique_ptr` — exclusive ownership

```cpp
#include <memory>

auto enemy = std::make_unique<Enemy>("Goblin");
enemy->update();
// freed automatically when unique_ptr dies
```

**Use case:** single owner; factory return types; pimpl.

### `std::shared_ptr` — shared ownership

```cpp
auto a = std::make_shared<MeshData>();
auto b = a; // ref count 2
```

**Use case:** shared read-only resources. Prefer `unique_ptr` when possible (cheaper, clearer).

### `std::weak_ptr`

Breaks cycles; observe a `shared_ptr` without keeping it alive.

---

## 11. Structs and classes (OOP)

### struct vs class

In C++, the only language difference is default access (`public` vs `private`). Convention:

- **`struct`** → plain data aggregates  
- **`class`** → invariants + behavior  

```cpp
struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;
};

class BankAccount {
public:
    explicit BankAccount(double opening);
    void deposit(double amount);
    [[nodiscard]] double balance() const;
private:
    double m_balance = 0.0;
};
```

### Members

```cpp
account.deposit(50.0);
double b = account.balance();
```

### `this`

```cpp
void BankAccount::deposit(double amount) {
    this->m_balance += amount; // optional; implied
}
```

### When to use what

| Prefer | When |
|--------|------|
| `struct` + public fields | Simple POD / data transfer |
| `class` + private data | Must protect invariants (balance ≥ 0, valid IDs) |
| Free functions | Stateless utilities (`lerp`, `clamp`) |
| Namespace | Group related free functions/types |

---

## 12. Encapsulation, abstraction, inheritance, polymorphism

### Encapsulation

Hide data; expose controlled operations.

```cpp
class Temperature {
public:
    void setCelsius(float c) {
        if (c < -273.15f) throw std::invalid_argument("below absolute zero");
        m_celsius = c;
    }
    float celsius() const { return m_celsius; }
private:
    float m_celsius = 20.0f;
};
```

**Use case:** prevent invalid states.

### Abstraction

Show *what*, hide *how*.

```cpp
class Logger {
public:
    virtual ~Logger() = default;
    virtual void info(const std::string& msg) = 0;
};

class ConsoleLogger : public Logger {
public:
    void info(const std::string& msg) override {
        std::cout << msg << '\n';
    }
};
```

User code depends on `Logger`, not console details.

**Use case:** swap file logger / network logger without rewriting callers.

### Inheritance

```cpp
class Animal {
public:
    virtual ~Animal() = default;
    virtual void speak() const = 0;
};

class Dog : public Animal {
public:
    void speak() const override { std::cout << "Woof\n"; }
};
```

### Polymorphism

Same interface, different behavior at runtime:

```cpp
void makeSound(const Animal& a) {
    a.speak();
}

Dog d;
makeSound(d);
```

Needs **virtual** functions + usually virtual destructor.

### Composition over inheritance

```cpp
class Car {
    Engine engine; // has-a, often better than is-a deep trees
};
```

**Use case:** prefer “has-a” unless you truly need an “is-a” hierarchy.

### SOLID (practical reading)

| Letter | Idea | Example |
|--------|------|---------|
| **S** | Single responsibility | Parser ≠ Renderer |
| **O** | Open for extension | New logger type without editing callers |
| **L** | Liskov | Subtypes honor base contracts |
| **I** | Interface segregation | Small focused interfaces |
| **D** | Depend on abstractions | Depend on `Logger`, not `ConsoleLogger` |

---

## 13. Constructors, destructors, copy, move

### Constructor / destructor

```cpp
class Buffer {
public:
    explicit Buffer(std::size_t n) : m_data(new int[n]), m_size(n) {}
    ~Buffer() { delete[] m_data; }
private:
    int* m_data;
    std::size_t m_size;
};
```

`explicit` prevents accidental implicit conversions.

### Rule of five (owning classes)

If you define one of: destructor, copy ctor, copy assign, move ctor, move assign — review all five.

```cpp
Buffer(const Buffer&) = delete;
Buffer& operator=(const Buffer&) = delete;

Buffer(Buffer&& other) noexcept
    : m_data(other.m_data), m_size(other.m_size) {
    other.m_data = nullptr;
    other.m_size = 0;
}
```

**Use case:** unique ownership of resources (file handles, raw arrays, GPU IDs).

### Move vs copy

| Copy | Move |
|------|------|
| Duplicate state | Steal state, leave source empty/valid |
| Expensive for big data | Cheap ownership transfer |

```cpp
std::vector<int> a(1'000'000);
std::vector<int> b = std::move(a); // move; a empty but valid
```

---

## 14. const, static, constexpr, namespaces

### `const`

```cpp
const int maxLives = 3;
void print(const std::string& s); // won't modify s

class Player {
public:
    int health() const { return m_health; } // won't modify *this
private:
    int m_health = 100;
};
```

### `static`

```cpp
struct IdGen {
    static int next() { return ++s_counter; }
private:
    static int s_counter;
};
int IdGen::s_counter = 0;
```

- Static **member function**: no `this`  
- Static **member data**: one shared instance  
- Static **local**: initialized once  

### `constexpr`

```cpp
constexpr int square(int x) { return x * x; }
constexpr int k = square(4); // compile-time when possible
```

**Use case:** constants and simple functions usable at compile time.

### Namespaces

```cpp
namespace math {
float clamp(float v, float lo, float hi);
}

math::clamp(5.f, 0.f, 1.f);
```

**Use case:** avoid name collisions; organize libraries.

---

## 15. Templates and the STL

### Function templates

```cpp
template <typename T>
T maxValue(T a, T b) {
    return (a > b) ? a : b;
}

maxValue(3, 5);
maxValue(2.5, 1.1);
```

### Class templates

```cpp
template <typename T>
struct Pair {
    T first;
    T second;
};

Pair<int> p{1, 2};
```

### Essential STL containers

| Container | Use case |
|-----------|----------|
| `std::vector<T>` | Dynamic array (default choice) |
| `std::array<T,N>` | Fixed size on stack/object |
| `std::string` | Text |
| `std::unordered_map<K,V>` | Hash dictionary |
| `std::map<K,V>` | Sorted dictionary |
| `std::unique_ptr<T>` | Unique ownership |
| `std::optional<T>` | Value-or-empty |
| `std::filesystem::path` | File paths |

```cpp
std::vector<std::string> names;
names.push_back("Ada");
names.size();
names[0];
names.data(); // pointer to contiguous storage
```

### Algorithms

```cpp
#include <algorithm>
std::sort(names.begin(), names.end());
auto it = std::find(names.begin(), names.end(), "Ada");
```

**Use case:** don’t hand-roll loops when `<algorithm>` already expresses intent.

---

## 16. Error handling

### Return codes

```cpp
enum class Status { Ok, NotFound, IOError };
Status load(const std::string& path);
```

### Exceptions

```cpp
try {
    BankAccount account(-10.0); // throws
} catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
}
```

| Prefer exceptions when | Prefer error codes when |
|------------------------|-------------------------|
| Constructor failure | Expected frequent failures |
| Rare / fatal path | Real-time hot loops |
| Can’t return a valid object | Simple recoverable status |

### Assertions (debug)

```cpp
#include <cassert>
assert(index < size);
```

---

## 17. Optimization tips

### Correctness first

Measure before optimizing. Use a profiler when it matters.

### Cheap wins

| Tip | Why |
|-----|-----|
| Pass big objects as `const T&` | Avoid copies |
| Prefer `std::vector` reserve | Avoid realloc churn: `v.reserve(n)` |
| Move instead of copy when transferring | `std::move` |
| Avoid premature `shared_ptr` | Atomic refcount cost |
| Small functions in headers as `inline`/templates carefully | Balance compile time |
| Contiguous data (`vector`) | Cache-friendly |
| Reserve string capacity | Fewer allocations |

```cpp
std::vector<int> v;
v.reserve(10'000);
for (int i = 0; i < 10'000; ++i) v.push_back(i);
```

### What not to do early

- Hand-written linked lists “for speed”  
- Micro-optimizing unreadably  
- Virtual calls everywhere “just in case”  

### Memory tips

```cpp
// Bad: many tiny allocations
for (...) map[i] = new Node(...);

// Better: pool / vector of Node by value when possible
std::vector<Node> nodes;
```

### `noexcept` moves

Mark move constructors `noexcept` so containers can move on resize instead of falling back to copies.

---

## 18. Modern C++ habits (C++17)

| Habit | Example |
|-------|---------|
| `nullptr` not `NULL` | `T* p = nullptr;` |
| `override` on virtuals | `void f() override;` |
| `= delete` unused special members | No accidental copies |
| Range-for | `for (auto& x : v)` |
| `std::make_unique` | Safe allocation |
| Structured bindings | `auto [a,b] = pair;` |
| `if` with init | `if (auto it = m.find(k); it != m.end())` |
| `[[nodiscard]]` | Don’t ignore results |
| `enum class` | Scoped enums |

```cpp
enum class Color { Red, Green, Blue };
Color c = Color::Red;
```

```cpp
std::pair<int, int> dims{1920, 1080};
auto [w, h] = dims;
```

---

## 19. Practice exercises

1. Write `swap` using pointers, then using references.  
2. Implement a `dynamic_array<int>` with `new[]`/`delete[]`, then replace storage with `std::vector`.  
3. Make a `BankAccount` that rejects negative balances (encapsulation).  
4. Create `Shape` → `Circle`/`Rectangle` with virtual `area()` (polymorphism).  
5. Return a large `std::vector` from a function and confirm moves (print sizes after `std::move`).  
6. Build a word counter with `std::unordered_map<std::string, int>`.  
7. Write a RAII timer that prints elapsed seconds in its destructor.  
8. Convert a C-style API (`const char*`) to `std::string` safely.  
9. Mark a resource class non-copyable and movable.  
10. Profile a loop with vs without `vector::reserve`.

---

## 20. Quick reference

### Pass / return

| Situation | Prefer |
|-----------|--------|
| `int`, `float`, `bool` | By value |
| `std::string`, `vector`, big struct (read) | `const T&` |
| Need to modify caller’s object | `T&` |
| Optional absence | `std::optional<T>` or pointer |
| Unique ownership transfer | `std::unique_ptr<T>` / move |

### Ownership one-liner

> Whoever **creates** a resource is responsible for **releasing** it — or hand that duty to RAII.

### Include starters

```cpp
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <utility> // std::move, std::exchange
```

### Minimal class skeleton

```cpp
class Widget {
public:
    Widget();
    ~Widget();

    Widget(const Widget&) = delete;
    Widget& operator=(const Widget&) = delete;

    Widget(Widget&& other) noexcept;
    Widget& operator=(Widget&& other) noexcept;

    void update();
    [[nodiscard]] int id() const;

private:
    int m_id = 0;
};
```

---

*Master values vs addresses, lifetimes, and ownership. OOP then becomes a way to organize those ideas — not a substitute for them.*
