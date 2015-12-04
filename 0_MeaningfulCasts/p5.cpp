// Copyright (c) 2015 Vittorio Romeo
// License: AFL 3.0 | https://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <type_traits>
#include <cassert>
#include <iostream>
#include "qualifier_utils.hpp"

// Some legacy APIs may require extensive casting to `void*`, even using
// numbers, which are non-pointer objects. One example of such an API is OpenGL.

template <typename T>
constexpr auto to_void_ptr(T* x) noexcept
{
    return static_cast<copy_cv_qualifiers<void*, T>>(x);
}

template <typename T>
constexpr auto num_to_void_ptr(T&& x) noexcept
    -> std::enable_if_t<!std::is_pointer<T>{}, // .
        copy_cv_qualifiers<void, T>*>
{
    static_assert(sizeof(void*) >= sizeof(T), // .
        "Input type `T` must fit into `void*.");

    static_assert(std::is_arithmetic<std::decay_t<T>>{}, // .
        "Input type `T` must be arithmethic.");

    return reinterpret_cast<copy_cv_qualifiers<void*, T>>(x);
}

int main()
{
    reinterpret_cast<void*>(10);

    // Will not compile:
    // to_void_ptr(10);

    // Will compile:
    num_to_void_ptr(10);

    return 0;
}

// My advice:
//
// * Be as explicit as possible in your code. Wrapping existing functions in
// stricter ones, with more descriptive names, is always a good thing.
//
// * Always try to use the strictest cast available.
//
// * Use Boost if you can!

// My implementation of these "casts" is available here:
// https://github.com/SuperV1234/vrm_core