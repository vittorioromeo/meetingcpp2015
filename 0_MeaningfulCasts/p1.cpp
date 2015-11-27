// Copyright (c) 2015 Vittorio Romeo
// License: AFL 3.0 | https://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <type_traits>
#include <cassert>
#include <iostream>
#include <cmath>
#include "will_overflow.hpp"

// Casting is an essential operation for all projects, but can be a source of
// errors when used improperly.

// C++ improved upon C's "cast operator" by defining new, more strict and safer
// casts. (Do not use C-style casts!)

// We can follow the same philosophy when defining our new "meaningful" casts:
// they will be more strict and safer than C++'s default casts.

// In this short talk, we'll cover the implementation of the following type
// conversions:

// * number <-> number
// * `enum` <-> `enum`
// * number <-> `enum`
// * base <-> derived
// * aligned storage <-> inner type
// * `void*` <-> number

// Let's begin with "number type" to "number type" conversions.

// TODO: mention Boost

// To truly benefit from a custom numerical cast, what we need is a function
// that can detect overflow/underflow and invalid operations before they happen.

// Its implementation is out of the scope of this talk. In short, it tries to
// detect overflows/underflows before they happen and it uses the <cfenv> header
// to check validity of floating point operations.

// Having implemented a `will_overflow` function, we can now define our first
// "cast": `to_num.

template <typename TOut, typename TIn>
constexpr auto to_num(const TIn& x) noexcept
{
    // The first thing we have to do is check that the source and target types
    // satisfy the `std::is_arithmetic` type trait.
    static_assert(std::is_arithmetic<TOut>{} && std::is_arithmetic<TIn>{}, "");

    // Afterwards, we assert that the conversion will not underflow/overflow
    // thanks to our magic `will_overflow` function.
    assert((!impl::will_overflow<TOut, TIn>(x)));

    // We can finally use `static_cast` to convert the number.
    return static_cast<TOut>(x);
}

// And that's pretty much it!

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
        /*
            (void)to_num<unsigned char>(-1);
            (void)to_num<unsigned short>(-1);
            (void)to_num<unsigned int>(-1);
        */

        // Ok:
        (void)to_num<char>((short)std::numeric_limits<char>::max());
        (void)to_num<char>((short)std::numeric_limits<char>::min());
        (void)to_num<short>((int)std::numeric_limits<short>::max());
        (void)to_num<short>((int)std::numeric_limits<short>::min());
        (void)to_num<int>((long)std::numeric_limits<int>::max());
        (void)to_num<int>((long)std::numeric_limits<int>::min());

        // Run-time assertion:
        /*
            (void)to_num<char>((short)std::numeric_limits<char>::max() + 1);
            (void)to_num<char>((short)std::numeric_limits<char>::min() - 1);
            (void)to_num<short>((int)std::numeric_limits<short>::max() + 1);
            (void)to_num<short>((int)std::numeric_limits<short>::min() - 1);
            (void)to_num<int>((long)std::numeric_limits<int>::max() + 1);
            (void)to_num<int>((long)std::numeric_limits<int>::min() - 1);
        */

        // Ok:
        (void)to_num<float>(std::numeric_limits<float>::max());

        // Run-time assertion:
        /*
            (void)to_num<float>(std::numeric_limits<double>::max());
            (void)to_num<float>(NAN);
        */
    }

    return 0;
}

// Let's move onto `enum`-related casts.