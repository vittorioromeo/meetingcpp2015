// Copyright (c) 2015 Vittorio Romeo
// License: AFL 3.0 | https://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <type_traits>
#include <cassert>
#include <iostream>
#include <limits>
#include <cstdint>
#include <cfenv>

// Casting is essential in pretty much all projects.

// The least strict and "most powerful" cast we can use in C++ is the "C-style
// cast", called "cast operator" in the standard.

// In short, it tries to convert a value to a type specified by the user
// following these rules:

// * Try implicit conversions "as if by assignment".
//
// * Covert any integer to a pointer type.
//   (implementation-defined behavior for non-null values)
//
// * Convert any pointer type to an integer.
//   (may result in undefined behavior)
//
// * Covert a pointer type to another pointer type.
//   (may result in undefined behavior)
//   (also applies to function pointers)

// The rules get even more messy and dangerous when dealing with `const`,
// `volatile` and other qualifiers.

// Do not use C-style casts!

// Thankfully, C++ introduced some stricter and "more meaningful" casts:

// * `static_cast<T>`
// * `const_cast<T>`
// * `reinterpret_cast<T>`
// * `dynamic_cast<T>`

// By having stricter rules, these casts are safer to use, and the compiler can
// catch many mistakes and inxid casts.

// We can do better, though!
// We can define our own even stricter, more meaningful and safer casts.

// In this short talk, we'll cover the implementation of the following type
// conversions:

// * number <-> number
// * `enum` <-> `enum`
// * number <-> `enum`
// * base <-> derived
// * aligned storage <-> inner type
// * `void*` <-> number

// Let's begin with "number type" to "number type" conversions.

// These conversions are usually done by using `static_cast` - some mistakes,
// however, can happen because of type signedness differences.

// Our conversion functions will make sure that these mistakes can be catched.

// We will define a `to_num` function that allows us to convert number types.

// To do that, we will need to define an helper alias: `same_signedness`, that
// will exuate to `true` if both types have the same signedness.

// TODO:

/*
template <typename>
constexpr int number_rank{0};

template <>
constexpr int number_rank<char>{0};

template <>
constexpr int number_rank<unsigned char>{0};

template <>
constexpr int number_rank<short>{1};

template <>
constexpr int number_rank<unsigned short>{1};

template <>
constexpr int number_rank<int>{2};

template <>
constexpr int number_rank<unsigned int>{2};

template <>
constexpr int number_rank<long>{2};

template <>
constexpr int number_rank<unsigned long>{2};

template <>
constexpr int number_rank<long long>{3};

template <>
constexpr int number_rank<unsigned long long>{3};
*/

bool assert_fired{false};

#define FAKE_ASSERT(...)     \
    if(!(__VA_ARGS__))       \
    {                        \
        assert_fired = true; \
    }

void ensure_assert()
{
    assert(assert_fired);
    assert_fired = false;
}

namespace impl
{
    template <typename TOut, typename TIn>
    using can_hold = std::integral_constant<bool, sizeof(TOut) >= sizeof(TIn)>;

    template <typename TOut, typename TIn>
    using are_arithmethic = std::integral_constant<bool,
        std::is_arithmetic<TOut>{} && std::is_arithmetic<TIn>{}>;

    template <typename TOut, typename TIn>
    using same_signedness = std::integral_constant<bool,
        are_arithmethic<TIn, TOut>{} &&
            std::is_signed<TOut>{} == std::is_signed<TIn>{}>;

    // Sanity checks:
    static_assert(same_signedness<int, int>{}, "");
    static_assert(same_signedness<int, char>{}, "");
    static_assert(same_signedness<long, int>{}, "");
    static_assert(same_signedness<float, int>{}, "");
    static_assert(!same_signedness<int, std::size_t>{}, "");
    static_assert(!same_signedness<int, unsigned char>{}, "");
    static_assert(!same_signedness<unsigned long, int>{}, "");
    static_assert(!same_signedness<float, unsigned int>{}, "");
}

namespace impl
{
    template <typename TOut, typename TIn>
    constexpr bool will_overflow_impl(
        const TIn& x, std::false_type, std::false_type) noexcept
    {
        if(x > std::numeric_limits<TOut>::max() ||
            x < std::numeric_limits<TOut>::lowest())
        {
            return true;
        }

        std::feclearexcept(FE_OVERFLOW | FE_UNDERFLOW | FE_INVALID);

        TOut temp0(x);
        TIn temp1(x);
        temp0 = temp1;
        (void)temp0;
        (void)temp1;

        if(std::fetestexcept(FE_OVERFLOW | FE_UNDERFLOW | FE_INVALID))
        {
            return true;
        }

        // Sanity check.
        if(static_cast<TIn>(static_cast<TOut>(x)) != x)
        {
            return true;
        }

        return false;
    }

    template <typename TOut, typename TIn>
    constexpr bool will_overflow_impl(
        const TIn& x, std::true_type, std::false_type) noexcept
    {
        if((long double)x > (long double)std::numeric_limits<TOut>::max() ||
            (long double)x < (long double)std::numeric_limits<TOut>::lowest())
        {
            return true;
        }

        return false;
    }

    template <typename TOut, typename TIn>
    constexpr bool will_overflow_impl(
        const TIn& x, std::false_type, std::true_type) noexcept
    {
        if((long double)x > (long double)std::numeric_limits<TOut>::max() ||
            (long double)x < (long double)std::numeric_limits<TOut>::lowest())
        {
            return true;
        }

        return false;
    }

    template <typename TOut, typename TIn>
    constexpr bool will_overflow_impl(
        const TIn& x, std::true_type, std::true_type) noexcept
    {
        using out_lim = std::numeric_limits<TOut>;
        using intmax_lim = std::numeric_limits<intmax_t>;

        constexpr auto out_min(out_lim::min());
        constexpr auto out_max(out_lim::max());
        constexpr auto intmax_max(intmax_lim::max());

        auto x_as_intmax_t(static_cast<intmax_t>(x));
        auto x_as_uintmax_t(static_cast<uintmax_t>(x));

        constexpr auto out_min_as_intmax_t(static_cast<intmax_t>(out_min));
        constexpr auto out_max_as_intmax_t(static_cast<intmax_t>(out_max));

        constexpr auto intmax_max_as_uintmax_t(
            static_cast<uintmax_t>(intmax_max));

        constexpr auto out_max_as_uintmax_t(static_cast<uintmax_t>(out_max));

        if(std::is_signed<TOut>{})
        {
            // The output type is signed, the input type is unsigned, and the
            // "uintmax-casted input" is bigger than the "uintmax-casted
            // intmax".
            if(!std::is_signed<TIn>{} &&
                x_as_uintmax_t > intmax_max_as_uintmax_t)
            {
                return true;
            }

            // The output type is signed, the input type is unsigned, and the
            // "intmax-casted input" is smaller than the "intmax-casted output
            // minimum".
            if(x_as_intmax_t < out_min_as_intmax_t)
            {
                return true;
            }

            // The output type is signed, the input type is unsigned, and the
            // "intmax-casted input" is bigger than the "intmax-casted output
            // maximum".
            if(x_as_intmax_t > out_max_as_intmax_t)
            {
                return true;
            }

            return false;
        }

        // The output type is unsigned, and `x` is negative.
        if(x < 0)
        {
            return true;
        }

        // The output type is unsigned, and the "uintmax-casted input" is bigger
        // than the "uintmax-casted output maximum".
        if(x_as_uintmax_t > out_max_as_uintmax_t)
        {
            return true;
        }

        // Sanity check.
        if(static_cast<TIn>(static_cast<TOut>(x)) != x)
        {
            return true;
        }

        return false;
    }

    template <typename TOut, typename TIn>
    constexpr bool will_overflow(const TIn& x) noexcept
    {
        if(will_overflow_impl<TOut, TIn>(x,
               std::integral_constant<bool, std::is_integral<TOut>{}>{},
               std::integral_constant<bool, std::is_integral<TIn>{}>{}))
        {
            return true;
        }

        return false;
    }
}


template <typename TOut, typename TIn>
constexpr auto to_num(const TIn& x) noexcept
{
    static_assert(impl::are_arithmethic<TOut, TIn>{},
        "`to_num` only works on numerical types.");

    FAKE_ASSERT(!impl::will_overflow<TOut, TIn>(x));

    return static_cast<TOut>(x);
}


// Let's now implement two versions of the `to_num` function, that will be
// restricted thanks to `std::enable_if` and SFINAE.
/*
// The first version will deal with same-signedness types.
template <typename TOut, typename TIn>
constexpr auto to_num(const TIn& x) noexcept
    -> std::enable_if_t<std::is_arithmetic<TOut>{} &&
                            std::is_arithmetic<TIn>{} &&
                            impl::same_signedness<TOut, TIn>{},
        TOut>
{
    return static_cast<TOut>(x);
}

// The second version will deal with different-signedness types.
// We will use a runtime assert to catch conversions that change the "meaning"
// of the value.
template <typename TOut, typename TIn>
constexpr auto to_num(const TIn& x) noexcept -> std::enable_if_t<
    std::is_arithmetic<TOut>{} && std::is_arithmetic<TIn>{} &&
        !impl::same_signedness<TOut, TIn>{}> && std::is_signed,
    TOut>
{
    return static_cast<TOut>(x);
}
*/

template <typename TOut, typename TIn>
auto are_same_representation()
{
    return sizeof(std::decay_t<TOut>) == sizeof(std::decay_t<TIn>) &&
           impl::same_signedness<std::decay_t<TOut>, std::decay_t<TIn>>{} &&
           std::is_integral<TOut>{} == std::is_integral<TIn>{};
}

template <typename TOut, typename TIn>
void test_val(const TIn& x, bool should_fire = false)
{
    assert_fired = false;
    if(are_same_representation<TOut, TIn>()) return;

    (void)to_num<TOut>(x);

    if(should_fire)
        ensure_assert();
    else
        assert(!assert_fired);
}

template <typename TOut, typename TIn>
void test_type(bool should_fire = false)
{
    assert_fired = false;
    if(are_same_representation<TOut, TIn>()) return;

    (void)to_num<TOut>(std::numeric_limits<TIn>::lowest());
    (void)to_num<TOut>(std::numeric_limits<TIn>::max());
    (void)to_num<TOut>(static_cast<TIn>(0));
    (void)to_num<TOut>(static_cast<TIn>(1));
    (void)to_num<TOut>(static_cast<TIn>(2));

    if(should_fire)
        ensure_assert();
    else
        assert(!assert_fired);
}

void integral_tests()
{
    // Should not fire asserts:
    test_type<char, char>();
    test_type<int, char>();
    test_type<int, int>();
    test_type<long, int>();
    test_type<long, long>();
    test_type<long long, long>();
    test_type<long long, long long>();
    test_type<unsigned char, unsigned char>();
    test_type<unsigned int, unsigned char>();
    test_type<unsigned int, unsigned int>();
    test_type<unsigned long, unsigned int>();
    test_type<unsigned long, unsigned long>();
    test_type<unsigned long long, unsigned long>();
    test_type<unsigned long long, unsigned long long>();

    // Should fire asserts:
    test_type<char, int>(true);
    test_type<int, long>(true);
    test_type<long, long long>(true);
    test_type<char, long long>(true);
    test_type<char, long>(true);
    test_type<unsigned char, unsigned int>(true);
    test_type<unsigned int, unsigned long>(true);
    test_type<unsigned long, unsigned long long>(true);
    test_type<unsigned char, unsigned long long>(true);
    test_type<unsigned char, unsigned long>(true);

    // Should not fire asserts:
    test_type<int, unsigned char>();
    test_type<long, unsigned char>();
    test_type<long, unsigned short>();
    test_type<long, unsigned int>();
    test_type<long long, unsigned char>();
    test_type<long long, unsigned short>();
    test_type<long long, unsigned int>();
    // test_type<long long, unsigned long>();

    // Should fire asserts:
    test_type<unsigned long long, char>(true);
    test_type<unsigned long long, short>(true);
    test_type<unsigned long long, int>(true);
    test_type<unsigned long long, long>(true);
    test_type<char, unsigned char>(true);
    test_type<unsigned int, char>(true);
    test_type<unsigned long, char>(true);
    test_type<unsigned long, short>(true);
    test_type<unsigned long, int>(true);
    test_type<int, unsigned int>(true);
    test_type<long, unsigned long>(true);
    test_type<unsigned char, char>(true);
    test_type<unsigned int, int>(true);
    test_type<unsigned long, long>(true);
    test_type<int, unsigned long>(true);
    test_type<unsigned int, long>(true);
    test_type<char, unsigned int>(true);
    test_type<unsigned char, int>(true);

    // Should fire asserts:
    test_val<unsigned char>((char)-1, true);
    test_val<unsigned short>((short)-1, true);
    test_val<unsigned int>((int)-1, true);
    test_val<unsigned long>((long)-1, true);
    test_val<unsigned long long>((long long)-1, true);

    using namespace std;

    // Should not fire asserts:
    test_val<unsigned char>(numeric_limits<unsigned char>::max());
    test_val<unsigned char>(numeric_limits<unsigned char>::min());
    test_val<unsigned short>(numeric_limits<unsigned short>::max());
    test_val<unsigned short>(numeric_limits<unsigned short>::min());
    test_val<unsigned int>(numeric_limits<unsigned int>::max());
    test_val<unsigned int>(numeric_limits<unsigned int>::min());
    test_val<unsigned long>(numeric_limits<unsigned long>::max());
    test_val<unsigned long>(numeric_limits<unsigned long>::min());
    test_val<unsigned long long>(numeric_limits<unsigned long long>::max());
    test_val<unsigned long long>(numeric_limits<unsigned long long>::min());

    // Should fire asserts:
    test_val<unsigned char>(
        (unsigned short)numeric_limits<unsigned char>::max() + 1, true);
    test_val<unsigned char>(
        (unsigned short)numeric_limits<unsigned char>::min() - 1, true);
    test_val<unsigned short>(
        (unsigned int)numeric_limits<unsigned short>::max() + 1, true);
    test_val<unsigned short>(
        (unsigned int)numeric_limits<unsigned short>::min() - 1, true);
    test_val<unsigned int>(
        (unsigned long)numeric_limits<unsigned int>::max() + 1, true);
    test_val<unsigned int>(
        (unsigned long)numeric_limits<unsigned int>::min() - 1, true);
    test_val<unsigned long>(
        (unsigned long long)numeric_limits<unsigned long>::max() + 1, true);
    test_val<unsigned long>(
        (unsigned long long)numeric_limits<unsigned long>::min() - 1, true);
    test_val<unsigned long long>(
        numeric_limits<unsigned long long>::max() + 1, true);
    test_val<unsigned long long>(
        numeric_limits<unsigned long long>::min() - 1, true);

    // Should fire asserts:
    test_val<char>((short)numeric_limits<char>::max() + 1, true);
    test_val<char>((short)numeric_limits<char>::min() - 1, true);
    test_val<short>((int)numeric_limits<short>::max() + 1, true);
    test_val<short>((int)numeric_limits<short>::min() - 1, true);
    test_val<int>((long)numeric_limits<int>::max() + 1, true);
    test_val<int>((long)numeric_limits<int>::min() - 1, true);
    test_val<long>((long long)numeric_limits<long>::max() + 1, true);
    test_val<long>((long long)numeric_limits<long>::min() - 1, true);
    test_val<long long>(numeric_limits<long long>::max() + 1, true);
    test_val<long long>(numeric_limits<long long>::min() - 1, true);

    // Should not fire asserts:
    test_val<unsigned char>(
        (unsigned short)numeric_limits<unsigned char>::max());
    test_val<unsigned char>(
        (unsigned short)numeric_limits<unsigned char>::min());
    test_val<unsigned short>(
        (unsigned int)numeric_limits<unsigned short>::max());
    test_val<unsigned short>(
        (unsigned int)numeric_limits<unsigned short>::min());
    test_val<unsigned int>((unsigned long)numeric_limits<unsigned int>::max());
    test_val<unsigned int>((unsigned long)numeric_limits<unsigned int>::min());
    test_val<unsigned long>(
        (unsigned long long)numeric_limits<unsigned long>::max());
    test_val<unsigned long>(
        (unsigned long long)numeric_limits<unsigned long>::min());
    test_val<unsigned long long>(numeric_limits<unsigned long long>::max());
    test_val<unsigned long long>(numeric_limits<unsigned long long>::min());
}

void floating_tests()
{
    // Should not fire asserts:
    test_type<float, float>();
    test_type<double, double>();
    test_type<long double, long double>();

    // Should fire asserts:
    test_type<float, double>(true);
    test_type<double, long double>(true);

    // Should not fire asserts:
    test_val<float>(0.f);
    test_val<float>(0.0);
    test_val<float>(0.0l);
    test_val<double>(0.f);
    test_val<double>(0.0);
    test_val<double>(0.0l);
    test_val<long double>(0.f);
    test_val<long double>(0.0);
    test_val<long double>(0.0l);

    // Should not fire asserts:
    test_val<float>(1.f);
    test_val<float>(1.0);
    test_val<float>(1.0l);
    test_val<double>(1.f);
    test_val<double>(1.0);
    test_val<double>(1.0l);
    test_val<long double>(1.f);
    test_val<long double>(1.0);
    test_val<long double>(1.0l);

    // Should not fire asserts:
    test_val<float>(-1.f);
    test_val<float>(-1.0);
    test_val<float>(-1.0l);
    test_val<double>(-1.f);
    test_val<double>(-1.0);
    test_val<double>(-1.0l);
    test_val<long double>(-1.f);
    test_val<long double>(-1.0);
    test_val<long double>(-1.0l);

    // Should fire asserts:
    test_val<float>(std::numeric_limits<float>::max() +
                        std::numeric_limits<float>::epsilon(),
        true);
    test_val<float>(std::numeric_limits<float>::lowest() -
                        std::numeric_limits<float>::epsilon(),
        true);
    test_val<double>(std::numeric_limits<double>::max() +
                         std::numeric_limits<double>::epsilon(),
        true);
    test_val<double>(std::numeric_limits<double>::lowest() -
                         std::numeric_limits<double>::epsilon(),
        true);
    test_val<long double>(std::numeric_limits<long double>::max() +
                              std::numeric_limits<long double>::epsilon(),
        true);
    test_val<long double>(std::numeric_limits<long double>::lowest() -
                              std::numeric_limits<long double>::epsilon(),
        true);

    // Should not fire asserts:
    test_val<float>(std::numeric_limits<float>::max());
    test_val<float>(std::numeric_limits<float>::lowest());
    test_val<double>(std::numeric_limits<double>::max());
    test_val<double>(std::numeric_limits<double>::lowest());
    test_val<long double>(std::numeric_limits<long double>::max());
    test_val<long double>(std::numeric_limits<long double>::lowest());
}

void mixed_tests()
{
    // Should not fire asserts:
    test_type<float, int>();
    test_type<float, long>();
    test_type<float, long long>();
    test_type<double, int>();
    test_type<double, long>();
    test_type<double, long long>();
    test_type<long double, int>();
    test_type<long double, long>();
    test_type<long double, long long>();

    // Should fire asserts:
    test_type<char, float>(true);
    test_type<short, float>(true);
    test_type<int, float>(true);
    test_type<long, float>(true);
    test_type<char, double>(true);
    test_type<short, double>(true);
    test_type<int, double>(true);
    test_type<long, double>(true);
    test_type<char, long double>(true);
    test_type<short, long double>(true);
    test_type<int, long double>(true);
    test_type<long, long double>(true);

    // Should not fire asserts:
    test_val<float>((char)0);
    test_val<float>((char)-1);
    test_val<float>((char)1);
    test_val<float>((unsigned char)0);
    test_val<float>((unsigned char)-1);
    test_val<float>((unsigned char)1);
    test_val<float>((int)0);
    test_val<float>((int)-1);
    test_val<float>((int)1);
    test_val<float>((unsigned int)0);
    test_val<float>((unsigned int)-1);
    test_val<float>((unsigned int)1);

    // Should not fire asserts:
    test_val<double>((char)0);
    test_val<double>((char)-1);
    test_val<double>((char)1);
    test_val<double>((unsigned char)0);
    test_val<double>((unsigned char)-1);
    test_val<double>((unsigned char)1);
    test_val<double>((int)0);
    test_val<double>((int)-1);
    test_val<double>((int)1);
    test_val<double>((unsigned int)0);
    test_val<double>((unsigned int)-1);
    test_val<double>((unsigned int)1);

    // Should not fire asserts:
    test_val<float>(std::numeric_limits<char>::lowest());
    test_val<float>(std::numeric_limits<char>::max());
    test_val<float>(std::numeric_limits<unsigned char>::lowest());
    test_val<float>(std::numeric_limits<unsigned char>::max());
    test_val<float>(std::numeric_limits<int>::lowest());
    test_val<float>(std::numeric_limits<int>::max());
    test_val<float>(std::numeric_limits<unsigned int>::lowest());
    test_val<float>(std::numeric_limits<unsigned int>::max());
    test_val<float>(std::numeric_limits<long>::lowest());
    test_val<float>(std::numeric_limits<long>::max());
    test_val<float>(std::numeric_limits<unsigned long>::lowest());
    test_val<float>(std::numeric_limits<unsigned long>::max());
    test_val<double>(std::numeric_limits<char>::lowest());

    // Should not fire asserts:
    test_val<double>(std::numeric_limits<char>::max());
    test_val<double>(std::numeric_limits<unsigned char>::lowest());
    test_val<double>(std::numeric_limits<unsigned char>::max());
    test_val<double>(std::numeric_limits<int>::lowest());
    test_val<double>(std::numeric_limits<int>::max());
    test_val<double>(std::numeric_limits<unsigned int>::lowest());
    test_val<double>(std::numeric_limits<unsigned int>::max());
    test_val<double>(std::numeric_limits<long>::lowest());
    test_val<double>(std::numeric_limits<long>::max());
    test_val<double>(std::numeric_limits<unsigned long>::lowest());
    test_val<double>(std::numeric_limits<unsigned long>::max());

    // Should not fire asserts:
    test_val<char>(0.f);
    test_val<char>(1.f);
    test_val<char>(-1.f);
    test_val<char>(0.0);
    test_val<char>(1.0);
    test_val<char>(-1.0);
    test_val<char>(0.l);
    test_val<char>(1.l);
    test_val<char>(-1.l);

    // Should not fire asserts:
    test_val<int>(0.f);
    test_val<int>(1.f);
    test_val<int>(-1.f);
    test_val<int>(0.0);
    test_val<int>(1.0);
    test_val<int>(-1.0);
    test_val<int>(0.l);
    test_val<int>(1.l);
    test_val<int>(-1.l);

    // Should not fire asserts:
    test_val<long>(0.f);
    test_val<long>(1.f);
    test_val<long>(-1.f);
    test_val<long>(0.0);
    test_val<long>(1.0);
    test_val<long>(-1.0);
    test_val<long>(0.l);
    test_val<long>(1.l);
    test_val<long>(-1.l);

    // Should not fire asserts:
    test_val<unsigned char>(0.f);
    test_val<unsigned char>(1.f);
    test_val<unsigned char>(0.0);
    test_val<unsigned char>(1.0);
    test_val<unsigned char>(0.l);
    test_val<unsigned char>(1.l);

    // Should not fire asserts:
    test_val<unsigned int>(0.f);
    test_val<unsigned int>(1.f);
    test_val<unsigned int>(0.0);
    test_val<unsigned int>(1.0);
    test_val<unsigned int>(0.l);
    test_val<unsigned int>(1.l);

    // Should not fire asserts:
    test_val<unsigned long>(0.f);
    test_val<unsigned long>(1.f);
    test_val<unsigned long>(0.0);
    test_val<unsigned long>(1.0);
    test_val<unsigned long>(0.l);
    test_val<unsigned long>(1.l);
}

int main()
{
    integral_tests();
    floating_tests();
    mixed_tests();
    return 0;
}
