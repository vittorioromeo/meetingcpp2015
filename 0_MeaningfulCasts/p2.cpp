// Copyright (c) 2015 Vittorio Romeo
// License: AFL 3.0 | https://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <type_traits>
#include <cassert>
#include <iostream>
#include <cmath>
#include "will_overflow.hpp"

template <typename TOut, typename TIn>
constexpr auto to_num(const TIn& x) noexcept
{
    static_assert(std::is_arithmetic<TOut>{} && std::is_arithmetic<TIn>{}, "");
    assert((!impl::will_overflow<TOut, TIn>(x)));

    return static_cast<TOut>(x);
}

// When dealing with `enum` types, we may want to perform the following
// conversions:
// * `enum` to number
// * number to `enum`
// * `enum` to `enum`

// Every `enum` type has an underlying type that can be retrieved using
// `std::underlying_type_t`.

// The most general `enum` to number cast converts the `enum` value to a type
// convertible to its underlying type.
template <typename TOut, typename TIn>
constexpr auto from_enum(const TIn& x) noexcept
{
    // Make sure the input is an `enum`.
    static_assert(std::is_enum<TIn>{}, "");

    // Make sure that the input's underlying type is convertible to the desired
    // output type.
    static_assert(std::is_convertible<std::underlying_type_t<TIn>, TOut>{}, "");

    // Use `to_num` to catch eventual overflows/underflows when casting the
    // inner value.
    return to_num<TOut>(static_cast<std::underlying_type_t<TIn>>(x));
}

// One common operation is converting an `enum` to its underlying type - this is
// a special case of the previous function.
template <typename TIn>
constexpr auto from_enum(const TIn& x) noexcept
{
    return from_enum<std::underlying_type_t<TIn>, TIn>(x);
}

// Converting a number to an `enum` is another common operation - the underlying
// type of the desired enum type must be convertible to the input type.

// Since we're gonna use the `to_enum` functions to converts `enum` types to
// other `enum` types as well, we need to constrain the "number -> `enum`" case
// using SFINAE.
template <typename TOut, typename TIn>
constexpr auto to_enum(const TIn& x) noexcept -> std::enable_if_t<
    std::is_enum<TOut>{} && !std::is_enum<TIn>{} &&
        std::is_convertible<std::underlying_type_t<TOut>, TIn>{},
    TOut>
{
    return static_cast<TOut>(to_num<std::underlying_type_t<TOut>>(x));
}

// The last function implements `enum` to `enum` conversion.
template <typename TOut, typename TIn>
constexpr auto to_enum(const TIn& x) noexcept
    -> std::enable_if_t<std::is_enum<TOut>{} && std::is_enum<TIn>{}, TOut>
{
    // We need to make sure that the underlying types are convertible between
    // each other.
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
    // TODO:

    // `enum to default underlying type:
    {
        (void)static_cast<std::underlying_type_t<int_enum>>(int_enum::a);
        (void)from_enum(int_enum::a);

        (void)static_cast<std::underlying_type_t<uchar_enum>>(uchar_enum::a);
        (void)from_enum(uchar_enum::a);
    }

    // `enum` to numerical type:
    {
        // Uncaught mistake:
        (void)static_cast<unsigned int>(int_enum::a);

        // Run-time assertion:
        /*
            (void)from_enum<unsigned int>(int_enum::a);
        */
    }

    // Numerical type to `enum`:
    {
        (void)static_cast<int_enum>(-1);
        (void)to_enum<int_enum>(-1);

        // Uncaught mistake:
        (void)static_cast<uchar_enum>(-1);

        // Run-time assertion:
        /*
            (void) to_enum<uchar_enum>(-1);
        */
    }

    // `enum` to `enum`:
    {
        (void)static_cast<int_enum>(uchar_enum::a);
        (void)to_enum<int_enum>(uchar_enum::a);

        // Uncaught mistake:
        (void)static_cast<uchar_enum>(int_enum::a);

        // Run-time assertion:
        /*
            (void)to_enum<uchar_enum>(int_enum::a);
        */
    }
}
