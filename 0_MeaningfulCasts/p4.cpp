// Copyright (c) 2015 Vittorio Romeo
// License: AFL 3.0 | https://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <type_traits>
#include <cassert>
#include <iostream>
#include <memory>
#include "qualifier_utils.hpp"

// We'll define two functions to move in a class hierarchy: `to_base` and
// `to_derived`. They will wrap `static_cast`.

// The functions are intended to be used both for polymorphic and
// non-polymorphic hierarchies.

// We can detect whether or not an hierarchy is polymorphic at compile-time.
// If it is, we can use the following assertion:
// `dynamic_cast<target_type>(ptr) == ptr`.

// This will guarantee that `ptr` is actually holding the type we're trying to
// access, and will work properly even with multiple inheritance.

// Our implementation strategy will be as follows:
//
// 1) We'll start by defining a tag-dispatch-overloaded function that will
// assert the aforementioned `dynamic_cast` only for polymorphic types.
//
// 2) A general `hierarchy_cast` function will check pointer validity and that
// the types belong to the same hierarchy. It will call the previously defined
// tag-dispatched function, using `std::is_polymorphic`.
//
// 3) We'll define the interface functions, that will call `hierarchy_cast`
// after computing the correctly qualified return type.

namespace impl
{
    // This overload will be called when `std::is_polymorphic<T>` is `true`.
    template <typename TOut, typename T>
    constexpr void assert_correct_polymorphic(T* ptr, std::true_type) noexcept
    {
        assert(dynamic_cast<TOut>(ptr) == ptr);
    }

    // This overload will be called when `std::is_polymorphic<T>` is `false`.
    template <typename, typename T>
    constexpr void assert_correct_polymorphic(T*, std::false_type) noexcept
    {
    }

    // `TOut` will be computed by the callee.
    template <typename TDerived, typename TBase, typename TOut, typename T>
    constexpr decltype(auto) hierarchy_cast(T* ptr) noexcept
    {
        static_assert(std::is_base_of<TBase, TDerived>{}, // .
            "`TBase` is not a base class of `TDerived`.");

        // Sanity check.
        assert(ptr != nullptr);

        assert_correct_polymorphic<TOut>(ptr, std::is_polymorphic<TBase>{});
        return static_cast<TOut>(ptr);
    }
}

template <typename TDerived, typename TBase>
constexpr decltype(auto) to_derived(TBase* base) noexcept
{
    using result_type = copy_cv_qualifiers<TDerived, TBase>*;
    return impl::hierarchy_cast<TDerived, TBase, result_type>(base);
}

template <typename TBase, typename TDerived>
constexpr decltype(auto) to_base(TDerived* derived) noexcept
{
    using result_type = copy_cv_qualifiers<TBase, TDerived>*;
    return impl::hierarchy_cast<TDerived, TBase, result_type>(derived);
}



// Example: non-polymorphic casts for CRTP.

template <typename T>
struct crtp_base : T
{
    auto& as_derived()
    {
        return *to_derived<T>(this);
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
    // to_derived<circle>(b0)->draw();

    b0 = my_circle.get();
    to_derived<circle>(b0)->draw();

    // Run-time assertion:
    // to_derived<rectangle>(b0)->draw();
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