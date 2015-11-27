// Copyright (c) 2015 Vittorio Romeo
// License: AFL 3.0 | https://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <type_traits>
#include <cassert>
#include <iostream>
#include "qualifier_utils.hpp"

//

namespace impl
{
    template <typename TDerived, typename TBase, typename TOut, typename TPtr>
    constexpr decltype(auto) polymorphic_cast(TPtr&& ptr) noexcept
    {
        static_assert(std::is_base_of<TBase, TDerived>{}, "");

        // Extra sanity check.
        assert(ptr != nullptr);

        return static_cast<TOut>(ptr);
    }
}

template <typename TDerived, typename TBase>
constexpr decltype(auto) to_derived(TBase* base) noexcept
{
    using return_type = copy_cv_qualifiers<TDerived, TBase>*;
    return impl::polymorphic_cast<TDerived, TBase, return_type>(base);
}

template <typename TBase, typename TDerived>
constexpr decltype(auto) to_base(TDerived* derived) noexcept
{
    using return_type = copy_cv_qualifiers<TBase, TDerived>*;
    return impl::polymorphic_cast<TDerived, TBase, return_type>(derived);
}

template <typename TDerived, typename TBase>
constexpr decltype(auto) to_derived(TBase&& base) noexcept
{
    return *to_derived<TDerived>(&base);
}

template <typename TBase, typename TDerived>
constexpr decltype(auto) to_base(TDerived&& derived) noexcept
{
    return *to_base<TBase>(&derived);
}

struct base0
{
};
struct derived00 : base0
{
};
struct derived01 : base0
{
};

struct base1
{
};
struct derived10 : base1
{
};
struct derived11 : base1
{
};

int main()
{
    // Base to derived:
    {
        base0* d00 = new derived00{};
        base1* d10 = new derived10{};

        (void)static_cast<derived00*>(d00);
        (void)to_derived<derived00>(d00);

        // Compile-time assertion:
        /*
            (void) static_cast<derived00*>(d10);
            (void) to_derived<derived00>(d10);
        */

        // Compile-time assertion:
        // (void) to_base<derived00>(d00);
    }

    // Derived to base:
    {
        derived00* d00_heap = new derived00{};
        derived00 d00_stack{};

        (void)static_cast<base0>(*d00_heap);
        (void)to_base<base0>(d00_heap);

        // Potential slicing error:
        (void)static_cast<base0>(d00_stack);

        // Value is not copied, but only referenced.
        (void)to_base<base0>(d00_stack);
    }
}

// For the last code segment, we'll write casts to `void*`, which are required
// in some legacy APIs.