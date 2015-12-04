// Copyright (c) 2015 Vittorio Romeo
// License: AFL 3.0 | https://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <utility>
#include <iostream>
#include <type_traits>
#include <vrm/core/static_if.hpp>
#include <vrm/core/for_args.hpp>

// Let's take a look at some extra examples.

using namespace vrm::core;

// Example: print all values, separated by comma.
template <typename T, typename... Ts>
void print_all(T&& x, Ts&&... xs)
{
    std::cout << x;

    static_if(bool_v<(sizeof...(xs) > 0)>)
        .then([](auto&&... ys)
            {
                std::cout << ", ";
                print_all(FWD(ys)...);
            })
        .else_([](auto&&...)
            {
                std::cout << "\n";
            })(FWD(xs)...);
}

// Example: parse a sequence of "compile-time" tokens.
struct base_int_token
{
};

struct base_char_token
{
};

template <int TI>
struct int_token : std::integral_constant<int, TI>, base_int_token
{
};

template <char TC>
struct char_token : std::integral_constant<char, TC>, base_char_token
{
};

struct end_token
{
};

void example_tokens()
{
    for_args(
        [](auto tx)
        {
            static_if(std::is_base_of<base_int_token, decltype(tx)>{})

                // Handle `int` tokens.
                .then([](auto ty)
                    {
                        static_if(bool_v<ty % 2 == 0>)
                            .then([](auto)
                                {
                                    std::cout << "even int token\n";
                                })
                            .else_([](auto)
                                {
                                    std::cout << "odd int token\n";
                                })(ty);
                    })
                .else_if(std::is_base_of<base_char_token, decltype(tx)>{})

                // Handle `char` tokens.
                .then([](auto ty)
                    {
                        static_if(bool_v<(ty >= 'a' && ty <= 'z')>)
                            .then([](auto)
                                {
                                    std::cout << "lowercase char token\n";
                                })
                            .else_if(bool_v<(ty >= 'A' && ty <= 'Z')>)
                            .then([](auto)
                                {
                                    std::cout << "uppercase char token\n";
                                })
                            .else_([](auto)
                                {
                                    std::cout << "non-alpha char token\n";
                                })(ty);
                    })
                .else_([](auto)
                    {
                        std::cout << "unrecognized token\n";
                    })(tx);
        },
        int_token<2>{}, int_token<3>{}, char_token<'c'>{}, char_token<'C'>{});
}

int main()
{
    print_all(1, 2, 3, 4);
    example_tokens();

    return 0;
}

// `static_if` can be an useful tool, but it's not a replacement for explicit
// template specialization.

// `static_if` can make code more readable and easier to understand when a small
// amount of instructions have to be executed depending on some compile-time
// predicate.

// Specializations, especially when "Concepts Lite" will become widespread,
// should be used when most of the code changes depending on a type/predicate.

// Combining `static_if` with existing and future constructs can result in
// readable and flexible generic code. Example from N4461:

/*
template <typename T, typename U> void f(T, U)
  requires C1<T> && (C2<U> || C3<U>)
{
    static_if (C2<U>)
    {
    }
    else if (C3<U>)
    {
    }
}
*/

// Some interesting discussion about the subject can be found on this reddit
// thread:
// https://reddit.com/r/cpp/comments/3q0nrn

// My implementation of `static_if` is available here:
// https://github.com/SuperV1234/vrm_core

// If you want a production-ready, mature and extremely powerful C++14 library,
// I recommend these:

// * "boost::hana", by Louis Dionne
//   https://github.com/boostorg/hana
//
//   (Modern metaprogramming library, making use of "type-value
//   encoding"/"dependent typing".)
//
//   "hana" provides the `if_` and `while_` constructs, that can be used to for
//   compile-time branching and loops.

// * "Fit", by Paul Fultz II
//   https://github.com/pfultz2/Fit
//
//   (Fit is a header-only C++11/C++14 library that provides utilities for
//   functions and function objects.)
//
//   "Fit" provides the `fit::conditional` construct, allowing users to create
//   powerful conditional overloads, which can be used as `static_if`-like
//   compile-time branches.
