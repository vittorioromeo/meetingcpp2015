// Copyright (c) 2015 Vittorio Romeo
// License: AFL 3.0 | https://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <type_traits>
#include <cassert>
#include <iostream>
#include "qualifier_utils.hpp"

// I find myself using `std::aligned_storage` quite often.
// It is a convenient type alias for a `struct` big enough to contain a specific
// type with correct alignment.

// The correct way to get the "contents" of the aligned storage is by using
// `reinterpret_cast`, which is unsafe and can lead to errors.

// Alternatively, two chained `static_cast` calls can be used, but its
// equivalent to the `reinterpret_cast` one.
// More information: stackoverflow.com/questions/19300142/

// When casting an aligned storage to a specific type `T`, we can actually check
// the validity of the storage at compile-time by comparing its size and
// alignment to the ones of `T`.

template <typename T, typename TStorage>
constexpr decltype(auto) storage_cast(TStorage* storage) noexcept
{
    static_assert(sizeof(typename TStorage::type) >= sizeof(T), // .
        "`TStorage` is not big enough for `T`.");

    static_assert(alignof(typename TStorage::type) >= alignof(T), // .
        "`TStorage` is not properly aligned for `T`.");

    // Extra sanity check.
    assert(storage != nullptr);

    // To avoid reimplementing this function for all possible qualifier
    // combinations, we use a simple `copy_cv_qualifiers` type-trait-like alias
    // that applies the qualifiers of a source type to another type.
    using return_type = copy_cv_qualifiers<T, TStorage>;

    // Lastly, we `reinterpret_cast`.
    return reinterpret_cast<return_type*>(storage);
}

int main()
{
    // Simple mistakes can be prevented at compile-time:
    {
        std::aligned_storage_t<sizeof(int), alignof(int)> s;

        *(reinterpret_cast<int*>(&s)) = 10;
        *(storage_cast<int>(&s)) = 10;
        assert(*storage_cast<int>(&s) == 10);

        // Uncaught mistake:
        *(reinterpret_cast<double*>(&s)) = 10.f;

        // Compile-time assertion:
        /*
            *(storage_cast<double>(&s)) = 10.f;
        */
    }

    // "Placement new" works as well:
    {
        std::aligned_storage_t<sizeof(int), alignof(int)> s;
        new(storage_cast<int>(&s)) int{10};

        assert(*(storage_cast<int>(&s)) == 10);
    }

    return 0;
}

// In the next code segment, we'll implement casts that allow us to move up and
// down a class hierarchy.