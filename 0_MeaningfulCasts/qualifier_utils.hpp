// Copyright (c) 2015 Vittorio Romeo
// License: AFL 3.0 | https://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <type_traits>
#include <cassert>
#include <iostream>

template <typename T, typename TSource>
using copy_const_qualifier = std::conditional_t<std::is_const<TSource>{}, // .
    std::add_const_t<T>, T>;

template <typename T, typename TSource>
using copy_volatile_qualifier =
    std::conditional_t<std::is_volatile<TSource>{}, // .
        std::add_volatile_t<T>, T>;

template <typename T, typename TSource>
using copy_cv_qualifiers =
    copy_const_qualifier<copy_volatile_qualifier<T, TSource>, TSource>;
