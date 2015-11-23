// Copyright (c) 2015 Vittorio Romeo
// License: AFL 3.0 | https://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <type_traits>
#include <cassert>
#include <iostream>
#include "qualifier_utils.hpp"

template <typename T, typename TStorage>
using valid_storage = std::integral_constant<bool,
    sizeof(typename TStorage::type) >= sizeof(T) &&
        alignof(typename TStorage::type) >= alignof(T)>;

/// @brief Wrapper around `reinterpret_cast`, intended for use with aligned
/// storages. Returns a pointer.
template <typename T, typename TStorage>
constexpr decltype(auto) from_storage(TStorage* storage) noexcept
{
    static_assert(valid_storage<T, TStorage>{}, "");
    assert(storage != nullptr);

    using return_type = copy_cv_qualifiers<T, TStorage>;
    return reinterpret_cast<return_type*>(storage);
}

/// @brief Wrapper around `reinterpret_cast`, intended for use with aligned
/// storages. Returns a reference.
template <typename T, typename TStorage>
constexpr decltype(auto) from_storage(TStorage& storage) noexcept
{
    return *from_storage<T>(&storage);
}

int main()
{
    {
        std::aligned_storage_t<sizeof(int), alignof(int)> s;

        reinterpret_cast<int&>(s) = 10;
        from_storage<int>(s) = 10;

        // Uncaught mistake:
        reinterpret_cast<double&>(s) = 10.f;

        // Run-time assertion:
        /*
            from_storage<double>(s) = 10.f;
        */
    }
}
