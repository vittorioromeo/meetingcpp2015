// Copyright (c) 2015 Vittorio Romeo
// License: AFL 3.0 | https://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <type_traits>
#include <cassert>
#include <iostream>
#include <cmath>
#include "will_overflow.hpp"

// TODO:
// * enum vs enum class
// * underlying type

template <typename TOut, typename TIn>
constexpr auto to_num(const TIn& x) noexcept
{
    static_assert(std::is_arithmetic<TOut>{} && std::is_arithmetic<TIn>{}, "");
    assert((!impl::will_overflow<TOut, TIn>(x)));

    return static_cast<TOut>(x);
}

/// @brief Converts an enum to a type convertible to its underlying type.
template <typename TOut, typename TIn>
constexpr auto from_enum(const TIn& x) noexcept
{
    static_assert(std::is_enum<TIn>{}, "");
    static_assert(std::is_convertible<std::underlying_type_t<TIn>, TOut>{}, "");

    return to_num<TOut>(static_cast<std::underlying_type_t<TIn>>(x));
}

/// @brief Converts an enum to its underlying type.
template <typename TIn>
constexpr auto from_enum(const TIn& x) noexcept
{
    return from_enum<std::underlying_type_t<TIn>, TIn>(x);
}

/// @brief Converts a number to an enum.
template <typename TOut, typename TIn>
constexpr auto to_enum(const TIn& x) noexcept -> std::enable_if_t<
    std::is_enum<TOut>{} && !std::is_enum<TIn>{} &&
        std::is_convertible<std::underlying_type_t<TOut>, TIn>{},
    TOut>
{
    return static_cast<TOut>(to_num<std::underlying_type_t<TOut>>(x));
}

/// @brief Converts an enum to another enum.
/// @details The underlying types must be convertible between each other.
template <typename TOut, typename TIn>
constexpr auto to_enum(const TIn& x) noexcept
    -> std::enable_if_t<std::is_enum<TOut>{} && std::is_enum<TIn>{}, TOut>
{
    static_assert(std::is_convertible<std::underlying_type_t<TOut>,
                      std::underlying_type_t<TIn>>{},
        "");

    return to_enum<TOut>(from_enum(x));
}

enum class int_enum : int
{
    a = -1,
    b,
    c
};

enum class uchar_enum : unsigned char
{
    a,
    b,
    c
};

int main()
{
    (void)static_cast<int>(int_enum::a);
    (void)from_enum(int_enum::a);

    (void)static_cast<float>(int_enum::a);
    (void)from_enum<float>(int_enum::a);

    (void)static_cast<char>(int_enum::a);
    (void)from_enum<char>(int_enum::a);

    (void)static_cast<int_enum>(uchar_enum::a);
    (void)to_enum<int_enum>(uchar_enum::a);

    (void)static_cast<uchar_enum>(int_enum::a);
    // (void)to_enum<uchar_enum>(int_enum::a);

    (void)static_cast<int>(uchar_enum::a);
    (void)from_enum<int>(uchar_enum::a);

    (void)static_cast<unsigned char>(int_enum::a);
    // (void)from_enum<unsigned char>(int_enum::a);    
}
