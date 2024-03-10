#pragma once

#include <algorithm>
#include <array>
#include <string_view>
#include <type_traits>
#include <optional>
#include <limits>

#define ENUM_REFL_IMPL(Friend, Enum, Type, ...)                                                  \
    enum class Enum : Type { __VA_ARGS__ };                                                      \
                                                                                                 \
    struct enr_##Enum##_container {                                                              \
        enr_##Enum##_container() = delete;                                                       \
                                                                                                 \
        static constexpr std::size_t entries_n = ::enr::detail::number_of_entries(#__VA_ARGS__); \
                                                                                                 \
        static constexpr auto const in_order_array =                                             \
            ::enr::detail::build_in_order_array<Enum, entries_n>(#__VA_ARGS__);                  \
                                                                                                 \
        static constexpr auto const enum_to_string =                                             \
            ::enr::detail::build_enum_to_string_array<Enum, entries_n>(in_order_array);          \
                                                                                                 \
        static constexpr auto const string_to_enum =                                             \
            ::enr::detail::build_string_to_enum_array<Enum, entries_n>(in_order_array);          \
    };                                                                                           \
    /* Hook to find the class information container through ADL. */                              \
    Friend enr_##Enum##_container* enr_hook(Enum)

#define ENUM_REFL(Enum, Type, ...) ENUM_REFL_IMPL(, Enum, Type, __VA_ARGS__)
#define ENUM_REFL_NESTED(Enum, Type, ...) ENUM_REFL_IMPL(friend, Enum, Type, __VA_ARGS__)

namespace enr {
namespace detail {

// This value is completely arbitrary and backed by no testing.
inline constexpr std::size_t binary_search_limit = 8;

inline consteval std::size_t number_of_entries(std::string_view enum_def)
{
    std::size_t ret = 0;
    bool found_something = false;
    for (auto c : enum_def) {
        switch (c) {
        case ',':
            ++ret;
            found_something = false;
            break;
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            break;
        default:
            found_something = true;
        }
    }
    return ret + found_something;
}

template<typename Int>
inline consteval Int decode_value(std::string_view value, bool negative)
{
    // This function assumes that the input is at least a valid c++ expression. The compiler should
    // have validated that before this function gets called.

    auto const trim = [](std::string_view v) -> std::string_view {
        auto const start = v.find_first_not_of(" \t\r\n");
        if (start == v.npos) {
            return "";
        }
        auto const end = v.find_last_not_of(" \t\r\n");
        return v.substr(start, end - start + 1);
    };

    value = trim(value);

    if (value.empty()) {
        throw "Got empty string value";
    }

    if (value[0] == '-') {
        return decode_value<Int>(value.substr(1), !negative);
    }

    if (value[0] == '+') {
        return decode_value<Int>(value.substr(1), negative);
    }

    // Search end of the numeric value (excluding suffix).
    auto const n_end = value.find_first_not_of("0123456789abcdefABCDEFxX");

    // Check that the suffix is indeed a "somewhat" valid suffix.
    if (n_end != value.npos) {
        auto const suffix = value.substr(n_end);
        if (std::ranges::find_if_not(
                value.substr(n_end),
                [](char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); })
            != suffix.cend()) {
            throw "Only integer literals are supported as enum values";
        }
    }

    // Discard suffix.
    value = value.substr(0, n_end);

    // Check that we actually have a numeric literal.
    if (value.empty() || value[0] < '0' || value[0] > '9') {
        throw "Only integer literals are supported as enum values";
    }

    Int base = 10;
    if (value[0] == '0') {
        base = 8;
        value = value.substr(1);
        if (value.empty()) {
            return 0;
        }
        if (value[0] == 'x' || value[0] == 'X') {
            base = 16;
            value = value.substr(1);
        } else if (value[0] == 'b' || value[0] == 'B') {
            base = 2;
            value = value.substr(1);
        } else if (value[0] < '0' || value[0] > '9') {
            throw "Unknown integer base";
        }
    }

    Int multiplier = negative ? -1 : 1;
    for (auto i = 0u; i < value.size() - 1; ++i) {
        multiplier *= base;
    }

    auto const get_value = [](char c) -> Int {
        if (c >= 'a') {
            c -= 0x20;
        }
        if (c >= 'A') {
            return c - 'A' + 0xa;
        }
        return c - '0';
    };

    Int ret = 0;
    for (auto c : value) {
        ret += get_value(c) * multiplier;
        multiplier /= base;
    }

    return ret;
}

template<typename Enum, std::size_t N>
inline consteval auto build_in_order_array(std::string_view enum_def)
{
    using Int = std::underlying_type_t<Enum>;

    std::array<std::pair<Enum, std::string_view>, N> array;

    std::size_t n = 0;
    Int value = 0;
    std::string_view substr = enum_def;

    auto const is_alnum_char = [](char c) {
        return c == '_' || c == '-' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
            || (c >= '0' && c <= '9');
    };

    auto const is_equals_or_comma = [](char c) { return c == '=' || c == ','; };

    std::string_view::const_iterator index;
    while ((index = std::ranges::find_if(substr, is_alnum_char)) != substr.cend()) {
        if (n >= N) {
            throw "Found more entries than expected";
        }

        substr = std::string_view{index, substr.cend()};

        // Look for identifier.
        index = std::ranges::find_if_not(substr, is_alnum_char);
        std::string_view const identifier{substr.cbegin(), index};
        substr = std::string_view{index, substr.cend()};

        auto const add_entry = [&] {
            array[n] = {Enum{value}, identifier};
            ++n;
            if (value != std::numeric_limits<Int>::max()) {
                ++value;
            }
        };

        // Look for equals or comma.
        index = std::ranges::find_if(substr, is_equals_or_comma);
        if (index == substr.cend()) {
            add_entry();
            break;
        }

        substr = std::string_view{index, substr.cend()};

        if (*index == ',') {
            add_entry();
            continue;
        }

        // Look for value.
        index = std::ranges::find_if(substr, is_alnum_char);
        if (index == substr.cend()) {
            throw "Expected value after equals";
        }

        substr = std::string_view{index, substr.cend()};
        index = std::ranges::find(substr, ',');
        std::string_view value_str{substr.cbegin(), index};

        value = decode_value<Int>(value_str, false);

        substr = std::string_view{index, substr.cend()};
        add_entry();
    }

    if (n < N) {
        throw "Found less entries than expected";
    }

    return array;
}

template<typename Enum, std::size_t N, typename Array>
inline consteval auto build_enum_to_string_array(Array&& arr)
{
    auto array = arr;
    std::ranges::sort(array, [](auto&& a, auto&& b) { return a.first < b.first; });
    return array;
}

template<typename Enum, std::size_t N, typename Array>
inline consteval auto build_string_to_enum_array(Array&& arr)
{
    auto array = arr;
    std::ranges::sort(array, [](auto&& a, auto&& b) { return a.second < b.second; });
    return array;
}

} // namespace detail

template<typename Enum>
concept enum_refl = requires(Enum e) { enr_hook(e); };

template<typename Enum>
struct is_enum_refl {
    static constexpr bool value = enum_refl<Enum>;
};

template<typename Enum>
inline constexpr auto is_enum_refl_v = is_enum_refl<Enum>::value;

template<enum_refl Enum>
inline constexpr auto const& entries()
{
    using Container = std::remove_pointer_t<decltype(enr_hook(Enum{}))>;
    return Container::in_order_array;
}

template<enum_refl Enum>
inline constexpr auto const& entries_by_value()
{
    using Container = std::remove_pointer_t<decltype(enr_hook(Enum{}))>;
    return Container::enum_to_string;
}

template<enum_refl Enum>
inline constexpr auto const& entries_by_name()
{
    using Container = std::remove_pointer_t<decltype(enr_hook(Enum{}))>;
    return Container::string_to_enum;
}

template<enum_refl Enum>
inline constexpr std::optional<std::string_view> to_string(Enum e)
{
    constexpr auto& array = entries_by_value<Enum>();

    if (array.size() >= detail::binary_search_limit) {
        auto const entry = std::ranges::lower_bound(
            array, e, [](auto a, auto b) { return a < b; }, [](auto&& a) { return a.first; });
        if (entry == array.cend() || entry->first != e) {
            return std::nullopt;
        }
        return entry->second;
    } else {
        auto const entry = std::ranges::find_if(array, [&](auto&& a) { return a.first == e; });
        if (entry == array.cend()) {
            return std::nullopt;
        }
        return entry->second;
    }
}

template<enum_refl Enum, typename String>
inline constexpr std::optional<Enum> to_enum(String&& s)
{
    std::string_view const sv{std::forward<String>(s)};
    constexpr auto& array = entries_by_name<Enum>();

    if (array.size() >= detail::binary_search_limit) {
        auto const entry = std::ranges::lower_bound(
            array, sv, [](auto a, auto b) { return a < b; }, [](auto&& a) { return a.second; });
        if (entry == array.cend() || entry->second != sv) {
            return std::nullopt;
        }
        return entry->first;
    } else {
        auto const entry = std::ranges::find_if(array, [&](auto&& a) { return a.second == sv; });
        if (entry == array.cend()) {
            return std::nullopt;
        }
        return entry->first;
    }
}

} // namespace enr
