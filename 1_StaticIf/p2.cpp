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



int main()
{
    static_if(std::is_integral<int>{})
        .then([](auto&&)
            {
                std::cout << "hi\n";
            })(1);
}