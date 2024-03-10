#include "enum_reflection.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <string>

using namespace std::literals;

namespace ns {

ENUM_REFL(TestEnum, uint32_t,
    TEST1 = 40u,
    TEST2 = 0b101ull,
    TEST3 = 11,
)

enum class NonEnumRefl {};

class Class {
    ENUM_REFL_NESTED(TestEnumNested, int,
        TEST1,
        TEST2 = 0123,
        TEST3,
    )

    static_assert(enr::is_enum_refl_v<TestEnumNested>);
    static_assert(enr::entries<TestEnumNested>()[0].second == "TEST1");
    friend void testPrivateEnum();
};

struct Struct {
    struct NestedStruct {
        ENUM_REFL_NESTED(TestEnumNested, int, TEST1, TEST2 = 0123, TEST3, )
        static_assert(enr::is_enum_refl_v<TestEnumNested>);
        static_assert(enr::entries<TestEnumNested>()[0].second == "TEST1");
    };
    static_assert(enr::is_enum_refl_v<NestedStruct::TestEnumNested>);
    static_assert(enr::entries<NestedStruct::TestEnumNested>()[0].second == "TEST1");
};

static_assert(enr::is_enum_refl_v<Struct::NestedStruct::TestEnumNested>);
static_assert(enr::entries<Struct::NestedStruct::TestEnumNested>()[0].second == "TEST1");

void testPrivateEnum()
{
    static_assert(enr::is_enum_refl_v<Class::TestEnumNested>);
    static_assert(enr::to_string(Class::TestEnumNested::TEST1) == "TEST1");
}

} // namespace ns

static_assert(enr::is_enum_refl_v<ns::TestEnum>);
static_assert(!enr::is_enum_refl_v<ns::NonEnumRefl>);

static_assert(static_cast<int>(ns::TestEnum::TEST1) == 40);

static_assert(enr::entries<ns::TestEnum>()[0].first == ns::TestEnum::TEST1);
static_assert(enr::entries<ns::TestEnum>()[1].first == ns::TestEnum::TEST2);
static_assert(enr::entries<ns::TestEnum>()[2].first == ns::TestEnum::TEST3);
static_assert(enr::entries<ns::TestEnum>().size() == 3);

static_assert(*enr::to_string(ns::TestEnum::TEST3) == "TEST3");
static_assert(*enr::from_string<ns::TestEnum>("TEST3") == ns::TestEnum::TEST3);

TEST(EnumRefl, Test)
{
    auto const non_existant_value = static_cast<ns::TestEnum>(123);
    auto const str = "TEST3"s;
    auto const value = enr::from_string<ns::TestEnum>(str);
    ASSERT_TRUE(value);
    ASSERT_EQ(*value, ns::TestEnum::TEST3);
    ASSERT_EQ(enr::from_string<ns::TestEnum>("nonexistant").value_or(non_existant_value),
              non_existant_value);
}
