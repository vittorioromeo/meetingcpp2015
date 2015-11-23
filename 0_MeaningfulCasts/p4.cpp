// Copyright (c) 2015 Vittorio Romeo
// License: AFL 3.0 | https://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <type_traits>
#include <cassert>
#include <iostream>
#include "qualifier_utils.hpp"

namespace impl
{
    template <typename TDerived, typename TBase, typename TOut, typename TPtr>
    constexpr decltype(auto) polymorphic_cast(TPtr&& ptr) noexcept
    {
        static_assert(std::is_base_of<TBase, TDerived>{}, "");
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

int main()
{
    // Base to derived:
    {

    }

    // Derived to base:
    {

    }
}
