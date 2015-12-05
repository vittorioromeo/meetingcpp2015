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
    static_assert(std::is_arithmetic<TOut>{}, // .
        "Output type `TOut` must be arithmetic.");

    static_assert(std::is_arithmetic<TIn>{}, // .
        "Input type `TIn` must be arithmetic.");

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
    static_assert(std::is_enum<TIn>{}, // .
        "Input type `TIn` must be an enum.");

    using underlying = std::underlying_type_t<TIn>;

    static_assert(std::is_convertible<underlying, TOut>{}, // .
        "`TIn`'s underlying type must be convertible to `TOut`.");

    // Use `to_num` to catch eventual errors when casting the inner value.
    return to_num<TOut>(static_cast<underlying>(x));
}

// One common operation is converting an `enum` to its own underlying type.
// This is a special case of the previous function.
template <typename TIn>
constexpr auto from_enum(const TIn& x) noexcept
{
    return from_enum<std::underlying_type_t<TIn>, TIn>(x);
}

// Converting a number to an `enum` is another common operation - the underlying
// type of the desired enum type must be convertible to the input type.

// Since we're gonna use the `to_enum` functions to converts `enum` types to
// other `enum` types as well, we need to constrain the "number -> `enum`" case
// using `std::enable_if_t`.

template <typename TOut, typename TIn>
constexpr auto to_enum(const TIn& x) noexcept -> std::enable_if_t< // .
    std::is_enum<TOut>{} && !std::is_enum<TIn>{},                  // .
    TOut>
{
    return static_cast<TOut>(to_num<std::underlying_type_t<TOut>>(x));
}

// Lastly, let's implement `enum` to `enum` conversion.
template <typename TOut, typename TIn>
constexpr auto to_enum(const TIn& x) noexcept -> std::enable_if_t< // .
    std::is_enum<TOut>{} && std::is_enum<TIn>{},                   // .
    TOut>
{
    return to_enum<TOut>(from_enum(x));
}

enum class int_enum : int
{
    neg0 = -1,
    pos0,
    pos1
};

enum class uchar_enum : unsigned char
{
    pos0,
    pos1,
    pos2
};

int main()
{
    // `enum to default underlying type:
    {
        static_cast<std::underlying_type_t<int_enum>>(int_enum::neg0);
        from_enum(int_enum::neg0);

        static_cast<std::underlying_type_t<uchar_enum>>(uchar_enum::pos0);
        from_enum(uchar_enum::pos0);
    }

    // `enum` to numerical type:
    {
        // `int_enum::neg0` is `-1`

        // Uncaught mistake:
        static_cast<unsigned int>(int_enum::neg0);

        // Run-time assertion:
        /*
            from_enum<unsigned int>(int_enum::neg0);
        */
    }

    // Numerical type to `enum`:
    {
        static_cast<int_enum>(-1);
        to_enum<int_enum>(-1);

        // Uncaught mistake:
        static_cast<uchar_enum>(-1);

        // Run-time assertion:
        /*
            to_enum<uchar_enum>(-1);
        */
    }

    // `enum` to `enum`:
    {
        // `int_enum::neg0` is `-1`

        static_cast<int_enum>(uchar_enum::pos0);
        to_enum<int_enum>(uchar_enum::pos0);

        // Uncaught mistake:
        static_cast<uchar_enum>(int_enum::neg0);

        // Run-time assertion:
        /*
            to_enum<uchar_enum>(int_enum::neg0);
        */
    }

    return 0;
}

// In the next code segment, we'll take a look at `std::aligned_storage`-related
// casts.