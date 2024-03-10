#include "enum_reflection.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <string>

using namespace std::literals;

#define NAME2(Name, Line) Name##Line
#define NAME(Name, Line) NAME2(Name, Line)

#define STATIC_ASSERTS(Enum, Type)                                             \
    static_assert(enr::enum_refl<Enum>);                                       \
    static_assert(enr::is_enum_refl_v<Enum>);                                  \
    static_assert(enr::is_enum_refl<Enum>::value);                             \
    static_assert(enr::entries<Enum>().size() == 3u);                          \
    static_assert(enr::entries<Enum>()[0] == std::pair{Enum::VAL1, "VAL1"sv}); \
    static_assert(enr::entries<Enum>()[1] == std::pair{Enum::VAL2, "VAL2"sv}); \
    static_assert(enr::entries<Enum>()[2] == std::pair{Enum::VAL3, "VAL3"sv}); \
    static_assert(enr::to_string(Enum::VAL1) == "VAL1");                       \
    static_assert(enr::to_string(Enum::VAL2) == "VAL2");                       \
    static_assert(enr::to_string(Enum::VAL3) == "VAL3");                       \
    static_assert(!enr::to_string(static_cast<Enum>(42)).has_value());         \
    static_assert(enr::to_enum<Enum>("VAL1") == Enum::VAL1);                   \
    static_assert(enr::to_enum<Enum>("VAL2") == Enum::VAL2);                   \
    static_assert(enr::to_enum<Enum>("VAL3") == Enum::VAL3);                   \
    static_assert(!enr::to_enum<Enum>("INVALID").has_value());

#define STATIC_TEST_ENUM_REFL_IMPL(Enum, Type, Value1, Value2, Value3) \
    namespace test_ns {                                                \
    ENUM_REFL(Enum, Type, \
        VAL1 Value1, \
        VAL2 Value2, \
        VAL3 Value3, \
    );    \
                                                                       \
    /* Test inside the same ns */                                      \
    STATIC_ASSERTS(Enum, Type)                                         \
    }                                                                  \
    /* Test outside the ns */                                          \
    STATIC_ASSERTS(test_ns::Enum, Type)

#define STATIC_TEST_ENUM_REFL_NESTED_IMPL(Class, Enum, Type, Value1, Value2, Value3) \
    namespace test_ns {                                                              \
    struct Class {                                                                   \
        ENUM_REFL_NESTED(Enum, Type, \
        VAL1 Value1, \
        VAL2 Value2, \
        VAL3 Value3, \
    );       \
        /* Test inside the class */                                                  \
        STATIC_ASSERTS(Enum, Type)                                                   \
    };                                                                               \
    /* Test inside the same ns */                                                    \
    STATIC_ASSERTS(Class::Enum, Type);                                               \
    }                                                                                \
    /* Test outside the ns */                                                        \
    STATIC_ASSERTS(test_ns::Class::Enum, Type)

#define STATIC_TEST_ENUM_REFL_NESTED(Type, Value1, Value2, Value3) \
    STATIC_TEST_ENUM_REFL_NESTED_IMPL(                             \
        NAME(Class_, __LINE__), NAME(Enum_, __LINE__), Type, Value1, Value2, Value3)

#define STATIC_TEST_ENUM_REFL(Type, Value1, Value2, Value3) \
    STATIC_TEST_ENUM_REFL_IMPL(NAME(Enum_, __LINE__), Type, Value1, Value2, Value3)

#define STATIC_TEST_ENUM_REFL_LIMITS_IMPL(Enum, Type, Value1, Value2, Value3)                  \
    STATIC_TEST_ENUM_REFL_IMPL(NAME(Enum_, __LINE__), Type, Value1, Value2, Value3)            \
    static_assert(static_cast<Type>(test_ns::Enum::VAL1) == std::numeric_limits<Type>::max()); \
    static_assert(static_cast<Type>(test_ns::Enum::VAL2) == std::numeric_limits<Type>::min()); \
    static_assert(static_cast<Type>(test_ns::Enum::VAL3) == std::numeric_limits<Type>::min() + 1);

#define STATIC_TEST_ENUM_REFL_LIMITS(Type, Value1, Value2, Value3) \
    STATIC_TEST_ENUM_REFL_LIMITS_IMPL(NAME(Enum_, __LINE__), Type, Value1, Value2, Value3)


STATIC_TEST_ENUM_REFL(int, , , )
STATIC_TEST_ENUM_REFL(int, = 10, , )
STATIC_TEST_ENUM_REFL(int, , = 10, )
STATIC_TEST_ENUM_REFL(int, = 10, = -10, )
STATIC_TEST_ENUM_REFL(int, = 3, = 2, = 1)
STATIC_TEST_ENUM_REFL(int, = 0x1234, = 01234, = 0b1010)
STATIC_TEST_ENUM_REFL(int, = -0x1234, = -01234, = -0b1010)
STATIC_TEST_ENUM_REFL(int, = +0x1234, = +01234, = +0b1010)
STATIC_TEST_ENUM_REFL(int, = 0x1234ll, = 01234ul, = 0b1010ull)
STATIC_TEST_ENUM_REFL(int, = -0x1234l, = -01234l, = -0b1010ll)
STATIC_TEST_ENUM_REFL(int, = +0x1234l, = +01234ul, = +0b1010ll)

STATIC_TEST_ENUM_REFL_NESTED(int, , , )
STATIC_TEST_ENUM_REFL_NESTED(int, = 10, , )
STATIC_TEST_ENUM_REFL_NESTED(int, , = 10, )
STATIC_TEST_ENUM_REFL_NESTED(int, = 10, = -10, )
STATIC_TEST_ENUM_REFL_NESTED(int, = 3, = 2, = 1)
STATIC_TEST_ENUM_REFL_NESTED(int, = 0x1234, = 01234, = 0b1010)
STATIC_TEST_ENUM_REFL_NESTED(int, = -0x1234, = -01234, = -0b1010)
STATIC_TEST_ENUM_REFL_NESTED(int, = +0x1234, = +01234, = +0b1010)
STATIC_TEST_ENUM_REFL_NESTED(int, = 0x1234ll, = 01234ul, = 0b1010ull)
STATIC_TEST_ENUM_REFL_NESTED(int, = -0x1234l, = -01234l, = -0b1010ll)
STATIC_TEST_ENUM_REFL_NESTED(int, = +0x1234l, = +01234ul, = +0b1010ll)

// Test limits.
STATIC_TEST_ENUM_REFL_LIMITS(uint8_t, = 0xff, = 0x00, )
STATIC_TEST_ENUM_REFL_LIMITS(uint8_t, = 0377, = 00, )
STATIC_TEST_ENUM_REFL_LIMITS(uint8_t, = 255, = 0, )

STATIC_TEST_ENUM_REFL_LIMITS(int8_t, = 0x7f, = -0x80, )
STATIC_TEST_ENUM_REFL_LIMITS(int8_t, = 0177, = -0200, )
STATIC_TEST_ENUM_REFL_LIMITS(int8_t, = 127, = -128, )

STATIC_TEST_ENUM_REFL_LIMITS(uint16_t, = 0xffff, = 0x00, )
STATIC_TEST_ENUM_REFL_LIMITS(uint16_t, = 0177777, = 00, )
STATIC_TEST_ENUM_REFL_LIMITS(uint16_t, = 65535, = 0, )

STATIC_TEST_ENUM_REFL_LIMITS(int16_t, = 0x7fff, = -0x8000, )
STATIC_TEST_ENUM_REFL_LIMITS(int16_t, = 077777, = -0100000, )
STATIC_TEST_ENUM_REFL_LIMITS(int16_t, = 32767, = -32768, )

STATIC_TEST_ENUM_REFL_LIMITS(uint32_t, = 0xffffffff, = 0x00, )
STATIC_TEST_ENUM_REFL_LIMITS(uint32_t, = 037777777777, = 00, )
STATIC_TEST_ENUM_REFL_LIMITS(uint32_t, = 4294967295, = 0, )

STATIC_TEST_ENUM_REFL_LIMITS(int32_t, = 0x7fffffff, = -0x80000000l, )
STATIC_TEST_ENUM_REFL_LIMITS(int32_t, = 017777777777, = -020000000000l, )
STATIC_TEST_ENUM_REFL_LIMITS(int32_t, = 2147483647, = -2147483648, )

STATIC_TEST_ENUM_REFL_LIMITS(uint64_t, = 0xffffffffffffffffull, = 0x00ull, )
STATIC_TEST_ENUM_REFL_LIMITS(uint64_t, = 01777777777777777777777ull, = 00ull, )
STATIC_TEST_ENUM_REFL_LIMITS(uint64_t, = 18446744073709551615ull, = 0, )

// int64_t kind of hard without having access to std::numeric_limits or expressions,
// -9223372036854775808ll is not a valid integer literal.

namespace test_ns {

// Empty (no entries)
ENUM_REFL(Test1, int, );
static_assert(enr::is_enum_refl_v<Test1>);
static_assert(enr::entries<Test1>().empty());

// Empty 2 (no entries, no trailing comma)
ENUM_REFL(Test2, int);
static_assert(enr::is_enum_refl_v<Test2>);
static_assert(enr::entries<Test2>().empty());

// Test type traits with a normal enum.
enum class Test3 {};
static_assert(!enr::is_enum_refl_v<Test3>);

// Test long enumeration (for binary search)
ENUM_REFL(Test4, int,
    E0 = -836664583,
    E1 = 1115269785,
    E2 = 931928927,
    E3 = 2041793061,
    E4 = -1555443174,
    E5 = 1117885838,
    E6 = 1488457881,
    E7 = -1208474023,
    E8 = -250021238,
    E9 = 415261831,
);
static_assert(enr::entries<Test4>().size() >= enr::detail::binary_search_limit);
static_assert(enr::to_string(Test4::E0) == "E0");
static_assert(enr::to_string(Test4::E1) == "E1");
static_assert(enr::to_string(Test4::E2) == "E2");
static_assert(enr::to_string(Test4::E3) == "E3");
static_assert(enr::to_string(Test4::E4) == "E4");
static_assert(enr::to_string(Test4::E5) == "E5");
static_assert(enr::to_string(Test4::E6) == "E6");
static_assert(enr::to_string(Test4::E7) == "E7");
static_assert(enr::to_string(Test4::E8) == "E8");
static_assert(enr::to_string(Test4::E9) == "E9");
static_assert(enr::to_enum<Test4>("E0") == Test4::E0);
static_assert(enr::to_enum<Test4>("E1") == Test4::E1);
static_assert(enr::to_enum<Test4>("E2") == Test4::E2);
static_assert(enr::to_enum<Test4>("E3") == Test4::E3);
static_assert(enr::to_enum<Test4>("E4") == Test4::E4);
static_assert(enr::to_enum<Test4>("E5") == Test4::E5);
static_assert(enr::to_enum<Test4>("E6") == Test4::E6);
static_assert(enr::to_enum<Test4>("E7") == Test4::E7);
static_assert(enr::to_enum<Test4>("E8") == Test4::E8);
static_assert(enr::to_enum<Test4>("E9") == Test4::E9);

} // namespace test_ns

TEST(EnumRefl, Runtime)
{
    auto const non_existant_value = static_cast<test_ns::Test4>(123);
    auto str = "E5"s;

    auto const value = enr::to_enum<test_ns::Test4>(str);
    ASSERT_TRUE(value);
    ASSERT_EQ(*value, test_ns::Test4::E5);

    str = "nonexistant";
    ASSERT_EQ(enr::to_enum<test_ns::Test4>(str).value_or(non_existant_value), non_existant_value);

    str = "E8";
    ASSERT_EQ(enr::to_string(test_ns::Test4::E8), str);

    auto const value2 = enr::to_string(non_existant_value);
    ASSERT_FALSE(value2);
}
