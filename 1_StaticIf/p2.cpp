// Copyright (c) 2015 Vittorio Romeo
// License: AFL 3.0 | https://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <utility>
#include <iostream>
#include <type_traits>

// Let's begin implementing "static if".
// Firstly, let's define some utility macros/functions.

#define FWD(...) ::std::forward<decltype(__VA_ARGS__)>(__VA_ARGS__)

template <bool TX>
using bool_ = std::integral_constant<bool, TX>;

// The interface function will be called `static_if` and will take a integral
// boolean constant as a parameter.

template <typename TPredicate>
auto static_if(TPredicate) noexcept;

// Our implementation will consist of a template struct that will be specialized
// with the predicate's result.

// When calling `static_if`, we will return a `static_if_impl` instance that
// will allow the user to build the branching logic.

// As soon as we find a matching branch, we will return a `static_if_result`
// instance, that will be propagated through the branching hierarchy and
// eventually call the correct function.

// Our design will be composed by the following types:
//
// * `static_if_impl`: will be returned by the `static_if` interface function.
// Every instance represents a branch. The type will be specialized depending on
// whether or not the predicate is matched.
//
// * `static_if_result`: will be returned when a matched `static_if_impl` is
// followed by a `then` continuation. This type will ignore any subsequent
// branches and overload `operator()` to allow the lambda inside the branch to
// be called.

namespace impl
{
    template <typename TFunctionToCall>
    struct static_if_result;

    template <bool TPredicateResult>
    struct static_if_impl;

    template <>
    struct static_if_impl<false>
    {
        template <typename TF>
        auto& then(TF&&)
        {
            // Ignore `then`, as the predicate is false.
            return *this;
        }

        template <typename TF>
        auto else_(TF&& f) noexcept
        {
            // Assuming that `else_` is after all `else_if` calls.

            // We found a matching branch, just make a result and ignore
            // everything else.

            return static_if_result<TF>(FWD(f));
        }

        template <typename TPredicate>
        auto else_if(TPredicate) noexcept
        {
            return static_if(TPredicate{});
        }

        template <typename... Ts>
        auto operator()(Ts&&...) noexcept
        {
            // If there are no `else` branches, we must ignore a call to a
            // failed static if matching.
        }
    };

    template <>
    struct static_if_impl<true>
    {
        template <typename TF>
        auto& else_(TF&&) noexcept
        {
            // Ignore `else_`, as the predicate is true.
            return *this;
        }

        template <typename TF>
        auto then(TF&& f) noexcept
        {
            // We found a matching branch, just make a result and ignore
            // everything else.

            return static_if_result<TF>(FWD(f));
        }

        template <typename TPredicate>
        auto& else_if(TPredicate) noexcept
        {
            // Ignore `else_if`, as the predicate is true.
            return *this;
        }
    };

    template <typename TFunctionToCall>
    struct static_if_result : TFunctionToCall
    {
        template <typename TFFwd>
        static_if_result(TFFwd&& f) noexcept : TFunctionToCall(FWD(f))
        {
        }

        template <typename TF>
        auto& else_(TF&&) noexcept
        {
            // Ignore everything, we found a result.
            return *this;
        }

        template <typename TF>
        auto& then(TF&&) noexcept
        {
            // Ignore everything, we found a result.
            return *this;
        }

        template <typename TPredicate>
        auto& else_if(TPredicate) noexcept
        {
            // Ignore everything, we found a result.
            return *this;
        }

        template <typename... Ts>
        decltype(auto) operator()(Ts&&... xs)
        {
            return (static_cast<TFunctionToCall&>(*this))(FWD(xs)...);
        }
    };
}

template <typename TPredicate>
auto static_if(TPredicate) noexcept
{
    return impl::static_if_impl<TPredicate{}>{};
}

// Now that we have a working implementation of `static_if`, let's analyze the
// previous example more closely.

struct banana
{
    void eat()
    {
    }
};

struct peanuts
{
    void eat()
    {
    }
};

struct water
{
    void drink()
    {
    }
};

struct juice
{
    void drink()
    {
    }
};

template <typename T>
constexpr bool is_solid{false};

template <>
constexpr bool is_solid<banana>{true};

template <>
constexpr bool is_solid<peanuts>{true};

template <typename T>
constexpr bool is_liquid{false};

template <>
constexpr bool is_liquid<water>{true};

template <>
constexpr bool is_liquid<juice>{true};

template <typename T>
auto consume(T&& x)
{
    // A `static_if_impl` instance is created here.
    // It is specialized on `true`/`false` depending on the predicate.
    static_if(bool_<is_solid<T>>{})

        // If `static_if_impl<true>` was instantiated, its `.then` method
        // returns a `static_if_result`, that will evaluate the branch thanks to
        // its `operator()`.

        // If `static_if_impl<false>` was instantiated, its `.then` method
        // will return `*this`, skipping the branch. The next method call will
        // be `.else_if`, which will return a new instance of `static_if_impl`,
        // evaluating the predicate.

        .then([](auto&& y)
            {
                y.eat();
                std::cout << "ate solid food\n";
            })
        .else_if(bool_<is_liquid<T>>{})
        .then([](auto&& y)
            {
                y.drink();
                std::cout << "drank liquid food\n";
            })

        // The same logic applies here - the difference is that the `.else_`
        // method will immediately return a `static_if_result` if it's being
        // called by `static_if_impl<false>`.

        .else_([](auto&&)
            {
                std::cout << "cannot consume\n";
            })

        // After walking down all branches, we'll either get a
        // `static_if_result`, which will call the first matched branch with its
        // `operator()`, or a `static_if_impl<false>`, which will do nothing in
        // its `operator()` implementation.

        (FWD(x));
}

int main()
{
    consume(banana{});
    consume(water{});
    consume(peanuts{});
    consume(juice{});
    consume(int{});
    consume(float{});

    return 0;
}