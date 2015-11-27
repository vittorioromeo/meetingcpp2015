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
    return static_cast<copy_cv_qualifiers<void, T>*>(x);
}

template <typename T>
constexpr auto num_to_void_ptr(T&& x) noexcept
    -> std::enable_if_t<!std::is_pointer<T>{}, copy_cv_qualifiers<void, T>*>
{
    static_assert(std::is_arithmetic<T>{}, "");
    return reinterpret_cast<copy_cv_qualifiers<void, T>*>(x);
}

int main()
{
    (void)(void*)10;

    // Will not compile:
    // (void)to_void_ptr(10);

    (void)num_to_void_ptr(10);
}
