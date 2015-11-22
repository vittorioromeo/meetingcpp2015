// Copyright (c) 2015 Vittorio Romeo
// License: AFL 3.0 | https://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <type_traits>
#include <cassert>
#include <iostream>
#include <cmath>
#include "will_overflow.hpp"

// Casting is essential in pretty much all projects.

// The least strict and "most powerful" cast we can use in C++ is the "C-style
// cast", called "cast operator" in the standard.

// In short, it tries to convert a value to a type specified by the user
// following these rules:

// * Try implicit conversions "as if by assignment".
//
// * Covert any integer to a pointer type.
//   (implementation-defined behavior for non-null values)
//
// * Convert any pointer type to an integer.
//   (may result in undefined behavior)
//
// * Covert a pointer type to another pointer type.
//   (may result in undefined behavior)
//   (also applies to function pointers)

// The rules get even more messy and dangerous when dealing with `const`,
// `volatile` and other qualifiers.

// Do not use C-style casts!

// Thankfully, C++ introduced some stricter and "more meaningful" casts:

// * `static_cast<T>`
// * `const_cast<T>`
// * `reinterpret_cast<T>`
// * `dynamic_cast<T>`

// By having stricter rules, these casts are safer to use, and the compiler can
// catch many mistakes and inxid casts.

// We can do better, though!
// We can define our own even stricter, more meaningful and safer casts.

// In this short talk, we'll cover the implementation of the following type
// conversions:

// * number <-> number
// * `enum` <-> `enum`
// * number <-> `enum`
// * base <-> derived
// * aligned storage <-> inner type
// * `void*` <-> number

// Let's begin with "number type" to "number type" conversions.

// TODO: will_overflow explanation

template <typename TOut, typename TIn>
constexpr auto to_num(const TIn& x) noexcept
{
    static_assert(std::is_arithmetic<TOut>{} && std::is_arithmetic<TIn>{}, "");
    assert((!impl::will_overflow<TOut, TIn>(x)));

    return static_cast<TOut>(x);
}

// And that's pretty much it!
// An assertion that checks for possible overflows could also be added.

int main()
{
    // Common usage scenario:
    {
        int a{10};
        (void)static_cast<float>(a);
        (void)to_num<float>(a);
    }

    // Dealing with enums:
    {
        enum E0
        {
            E0_a = 0,
            E0_b = 1
        };

        (void)static_cast<float>(E0::E0_a);

        // Does not compile, as intended:
        // (void)to_num<float>(E0::E0_a);

        // Explicit cast is required:
        (void)to_num<float>(static_cast<int>(E0::E0_a));
    }

    // Dealing with implicit conversions:
    {
        struct S0
        {
            operator int()
            {
                return 10;
            }
        };

        (void)static_cast<float>(S0{});

        // Does not compile, as intended:
        // (void) to_num<float>(S0{});

        // Explicit cast is required:
        (void)to_num<float>(static_cast<int>(S0{}));
    }

    // Catching overflows/underflows:
    {
        // Ok:
        (void)to_num<char>(-1);
        (void)to_num<short>(-1);
        (void)to_num<int>(-1);

        // Run-time assertion:
        // (void)to_num<unsigned char>(-1);
        // (void)to_num<unsigned short>(-1);
        // (void)to_num<unsigned int>(-1);

        // Ok:
        (void)to_num<char>((short)std::numeric_limits<char>::max());
        (void)to_num<char>((short)std::numeric_limits<char>::min());
        (void)to_num<short>((int)std::numeric_limits<short>::max());
        (void)to_num<short>((int)std::numeric_limits<short>::min());
        (void)to_num<int>((long)std::numeric_limits<int>::max());
        (void)to_num<int>((long)std::numeric_limits<int>::min());

        // Run-time assertion:
        // (void)to_num<char>((short)std::numeric_limits<char>::max() + 1);
        // (void)to_num<char>((short)std::numeric_limits<char>::min() - 1);
        // (void)to_num<short>((int)std::numeric_limits<short>::max() + 1);
        // (void)to_num<short>((int)std::numeric_limits<short>::min() - 1);
        // (void)to_num<int>((long)std::numeric_limits<int>::max() + 1);
        // (void)to_num<int>((long)std::numeric_limits<int>::min() - 1);

        // Ok:
        (void)to_num<float>(std::numeric_limits<float>::max());

        // Run-time assertion:
        // (void)to_num<float>(std::numeric_limits<double>::max());

        // Run-time assertion:
        // (void)to_num<float>(NAN);
    }

    return 0;
}

// Let's move onto `enum`-related casts.