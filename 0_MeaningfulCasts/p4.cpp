// Copyright (c) 2015 Vittorio Romeo
// License: AFL 3.0 | https://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <type_traits>
#include <cassert>
#include <iostream>
#include <memory>
#include "qualifier_utils.hpp"

// We'll define four functions to move in a class hierarchy:
// * `to_base`: from derived to base type, non-polymorphic.
// * `to_derived`: from base to derived type, non-polymorphic.
// * `to_polymorphic_base`: from derived to base type, polymorphic.
// * `to_polymorphic_derived`: from base to derived type, polymorphic.

// The non-polymorphic functions will simply wrap `static_cast` and check for
// pointer validity.

// The polymorphic functions will assert that:
// `dynamic_cast<target_type>(ptr) == ptr`.

// This will guarantee that the types are part of the same polymorphic
// hierarchy, and will work properly even with multiple inheritance.

// Our implementation strategy will be as follows:
//
// 1) We'll define a general `hierarchy_cast` function that will check for
// pointer validity and that the types belong to the same hierarchy.
//
// 2) We'll define the `to_base_impl` and `to_derived_impl` functions that will
// compute the correct return type and call `hierarchy_cast`.
//
// 3) We'll define the interface functions, that will check whether or not the
// type is polymorphic and perform the `dynamic_cast` check if appropriate.

namespace impl
{
    template <typename TDerived, typename TBase, typename TOut, typename T>
    constexpr decltype(auto) hierarchy_cast(T&& ptr) noexcept
    {
        static_assert(std::is_base_of<TBase, TDerived>{}, // .
            "`TBase` is not a base class of `TDerived`.");

        // Sanity check.
        assert(ptr != nullptr);

        return static_cast<TOut>(ptr);
    }

    template <typename TDerived, typename TBase>
    constexpr decltype(auto) to_derived_impl(TBase* base) noexcept
    {
        return hierarchy_cast<TDerived, TBase, // .
            copy_cv_qualifiers<TDerived, TBase>*>(base);
    }

    template <typename TBase, typename TDerived>
    constexpr decltype(auto) to_base_impl(TDerived* derived) noexcept
    {
        return hierarchy_cast<TDerived, TBase, // .
            copy_cv_qualifiers<TBase, TDerived>*>(derived);
    }
}

// We'll provide two overloads per function: the first one will take and return
// a pointer, the second one will take and return a reference.

template <typename TDerived, typename TBase>
constexpr decltype(auto) to_derived(TBase* base) noexcept
{
    static_assert(!std::is_polymorphic<TBase>{}, "");
    return impl::to_derived_impl<TDerived, TBase>(base);
}

template <typename TBase, typename TDerived>
constexpr decltype(auto) to_base(TDerived* derived) noexcept
{
    static_assert(!std::is_polymorphic<TBase>{}, "");
    return impl::to_base_impl<TBase, TDerived>(derived);
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

template <typename TDerived, typename TBase>
constexpr decltype(auto) to_polymorphic_derived(TBase* base) noexcept
{
    static_assert(std::is_polymorphic<TBase>{}, "");

    decltype(auto) res(impl::to_derived_impl<TDerived, TBase>(base));
    assert(dynamic_cast<decltype(res)>(base) == base);

    return res;
}

template <typename TBase, typename TDerived>
constexpr decltype(auto) to_polymorphic_base(TDerived* derived) noexcept
{
    static_assert(std::is_polymorphic<TBase>{}, "");

    decltype(auto) res(impl::to_base_impl<TBase, TDerived>(derived));
    assert(dynamic_cast<decltype(res)>(derived) == derived);

    return res;
}

template <typename TDerived, typename TBase>
constexpr decltype(auto) to_polymorphic_derived(TBase&& base) noexcept
{
    return *to_polymorphic_derived<TDerived>(&base);
}

template <typename TBase, typename TDerived>
constexpr decltype(auto) to_polymorphic_base(TDerived&& derived) noexcept
{
    return *to_polymorphic_base<TBase>(&derived);
}



// Example: non-polymorphic casts for CRTP.

template <typename T>
struct crtp_base : T
{
    auto& as_derived()
    {
        return to_derived<T>(*this);
    }

    void print()
    {
        as_derived().print_impl();
    }
};

struct ctrp_helloworld
{
    void print_impl()
    {
        std::cout << "hello world!\n";
    }
};



// Example: polymorphic casts in a shape hierarchy.

struct shape
{
    virtual ~shape()
    {
    }

    virtual void draw() = 0;
};

struct rectangle : shape
{
    void draw() override
    {
        std::cout << "draw rectangle\n";
    }
};

struct circle : shape
{
    void draw() override
    {
        std::cout << "draw circle\n";
    }
};

void shape_example()
{
    auto my_circle = std::make_unique<circle>();
    auto my_rectangle = std::make_unique<rectangle>();

    shape* b0{nullptr};

    // Run-time assertion:
    // to_polymorphic_derived<circle>(b0)->draw();

    b0 = my_circle.get();
    to_polymorphic_derived<circle>(b0)->draw();

    // Run-time assertion:
    // to_polymorphic_derived<rectangle>(b0)->draw();
}

int main()
{
    shape_example();
    return 0;
}

// Note that Boost provides a production-ready implementation,
// `boost::polymorphic_cast`, which allows users to move through class
// hierarchies both statically and dynamically.

// For the last code segment, we'll write casts to `void*`, which are required
// in some legacy APIs.