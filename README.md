# enum_reflection

Yet another macro-based header only library that provides reflection capabilities for enumerations
in C++. Some features:
- Written in standard C++20, no compiler or vendor extensions.
- Provides enumeration value iteration, number of elements, string to enum and enum to string.
- All preprocessing is done at compile time.
- All facilities are available at compile and runtime.
- Type traits and concepts to tell apart normal enumerations from reflection-enabled enumerations.
- Works for namespace-scope and class-scope enumerations.

## Example

```cpp
#include "enum_reflection.h"

ENUM_REFL(Enum /* enum name */, int /* Unerlying type */,
    A, B, C, D,
);

static_assert(enr::entries<Enum>().size() == 4);
static_assert(enr::to_string(Enum::B) == "B");
static_assert(enr::to_enum<Enum>("A") == Enum::A);

// Works for enumerations in any namespace.
namespace ns
{
ENUM_REFL(EnumNs, int,
    A, B, C, D,
);
} // namespace ns

static_assert(enr::entries<ns::EnumNs>().size() == 4);
static_assert(enr::to_string(ns::EnumNs::B) == "B");
static_assert(enr::to_enum<ns::EnumNs>("A") == ns::EnumNs::A);

// Also for nested enums in a class, using a different macro.
namespace ns
{
struct TestClass {
    ENUM_REFL_NESTED(EnumNested, int,
        A, B, C, D,
    );

    // Can use reflection machinery inside the class.
    static_assert(enr::entries<EnumNested>().size() == 4);
    static_assert(enr::to_string(EnumNested::B) == "B");
    static_assert(enr::to_enum<EnumNested>("A") == EnumNested::A);
};

// Also outside the class.
static_assert(enr::entries<TestClass::EnumNested>().size() == 4);
static_assert(enr::to_string(TestClass::EnumNested::B) == "B");
static_assert(enr::to_enum<TestClass::EnumNested>("A") == TestClass::EnumNested::A);

} // namespace ns

// And also outside the namespace.
static_assert(enr::entries<ns::TestClass::EnumNested>().size() == 4);
static_assert(enr::to_string(ns::TestClass::EnumNested::B) == "B");
static_assert(enr::to_enum<ns::TestClass::EnumNested>("A") == ns::TestClass::EnumNested::A);

// Explicit values are supported.
ENUM_REFL(EnumExplicit, int,
    A = 1, B = 42, C = -3, D,
);
static_assert(enr::entries<EnumExplicit>().size() == 4);
static_assert(enr::to_string(EnumExplicit::B) == "B");
static_assert(enr::to_enum<EnumExplicit>("D") == EnumExplicit::D);

// ... but with limitations, only numeric literals are supported.
inline constexpr int val = 0;
ENUM_REFL(EnumUnsupported, int,
    A = 1 + 2, // Expressions are unsupported.
    B = A,     // References to other entries are unsupported.
    C = val,   // References to constexpr variables or functions are unsupported.
);

// Example of iteration of entries.
#include <iostream>

ENUM_REFL(AnotherEnum, int,
    A, B, C, D,
);

void print_values()
{
    // entry.first contains the enum value, entry.second the string representation.
    for (auto const& entry : enr::entries<AnotherEnum>()) {
        std::cout << entry.second << '\n';
    }

    // entry.first can also be used, there is a provided operator ostream<<.
    for (auto const& entry : enr::entries<AnotherEnum>()) {
        std::cout << entry.first << '\n';
    }

    // The operator<< is robust against entries not present in the enumeration.
    std::cout << static_cast<AnotherEnum>(-1) << '\n'; // Will print "UNKOWN(-1)"
}

```

## How does it work

Esentially, the macro looks like this:
```cpp
#define ENUM_REFL(Enum, Type, ...)                                        \
    enum class Enum : Type { __VA_ARGS__ };                               \
    constexpr auto n_elements = compute_number_of_elements(#__VA_ARGS__); \
    constexpr auto entry_array = build_entry_array<Enum, n_elements>(#__VA_ARGS__);

    ENUM_REFL(EnumName, int, A = 1, B = 2, C = 3,);
```
The calls to `compute_number_of_elements()` and `build_entry_array()` receive the stringified
variadic arguments of the macro, so they just parse the string to build the metadata.

```cpp
compute_number_of_elements("A = 1, B = 2, C = 3,");
build_entry_array<EnumNamem n_elements>("A = 1, B = 2, C = 3,");
```
The `constexpr` values are actually stored as static members of a class, and they are found from any
namespace by abusing ADL.

## Limitations

- The biggest limitation is that it only support integer literals for explicit entry values, so no
  expressions, references to constant variables or constexpr functions. It's obvious why, given
  that the stringified argument list has to be parsed without having access to the compiler's
  context. Expressions with only integer literals or references to other enum values could be
  supported in principle, but they are not implemented.
- Cannot retrofit an existing enumeration, enumerations must be defined using the library macros
  to be supported.
