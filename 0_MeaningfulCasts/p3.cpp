// Copyright (c) 2015 Vittorio Romeo
// License: AFL 3.0 | https://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <type_traits>
#include <cassert>
#include <iostream>
#include "qualifier_utils.hpp"

// I find myself using `std::aligned_storage` quite often.
// It is a convenient struct big enough to contain a specific type with correct
// alignment.

// The correct way to get the "contents" of the aligned storage is by using
// `reinterpret_cast`, which is unsafe and can lead to errors.

// An alternative uses two chained `static_cast` calls, but its equivalent to
// the `reinterpret_cast` one.
// More information: stackoverflow.com/questions/19300142/

// When casting an aligned storage to a specific type `T`, we can actually check
// the validity of the storage at compile-time by comparing its size and
// alignment to the ones of `T`.

template <typename T, typename TStorage>
using valid_storage = std::integral_constant<bool,
    sizeof(typename TStorage::type) >= sizeof(T) &&
        alignof(typename TStorage::type) >= alignof(T)>;

// We can now define our conversion function `from_storage` that statically
// asserts the validity of the storage using `valid_storage`.

template <typename T, typename TStorage>
constexpr decltype(auto) from_storage(TStorage* storage) noexcept
{
    static_assert(valid_storage<T, TStorage>{}, "");

    // Extra sanity check.
    assert(storage != nullptr);

    // To avoid reimplementing this function for all possible qualifier
    // combinations, we use a simple `copy_cv_qualifiers` type-trait-like alias
    // that applies the qualifiers of a source type to another type.
    using return_type = copy_cv_qualifiers<T, TStorage>;

    // Lastly, we `reinterpret_cast`.
    return reinterpret_cast<return_type*>(storage);
}

// To improve ease of use, we can also define a version of the function that
// takes references.

template <typename T, typename TStorage>
constexpr decltype(auto) from_storage(TStorage& storage) noexcept
{
    return *from_storage<T>(&storage);
}

int main()
{
    // Simple mistakes can be prevented at compile-time:
    {
        std::aligned_storage_t<sizeof(int), alignof(int)> s;

        reinterpret_cast<int&>(s) = 10;
        from_storage<int>(s) = 10;

        // Uncaught mistake:
        reinterpret_cast<double&>(s) = 10.f;

        // Compile-time assertion:
        /*
            from_storage<double>(s) = 10.f;
        */
    }

    // "Placement new" works as well:
    {
        std::aligned_storage_t<sizeof(int), alignof(int)> s;
        new(from_storage<int>(&s)) int{10};

        assert(from_storage<int>(s) == 10);
    }
}

// In the next code segment, we'll implement casts that allow us to move up and
// down a polymorphic hierarchy.