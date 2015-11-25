// Copyright (c) 2015 Vittorio Romeo
// License: AFL 3.0 | https://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

// "static if" and similar constructs would allow C++ developers to create
// compile-time branches that enable different parts of code depending on a
// compile-time condition, using familiar "imperative" syntax.

// proposals:

// N3613: "Static If" considered
// open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3613.pdf

// N4461: Static if resurrected
// open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4461.html

// Examples from N4461
// Motivating example 1:

/*
template <class T>
void f(T&& t)
{
    // handle one T
}
template <class T, class... Rest>
void f(T&& t, Rest&&... r)
{
    f(t);
    // handle the tail
    f(r...);
}

// With static if:
template <class T, class... Rest>
void f(T&& t, Rest&&... r)
{
    //  handle one T

    static_if (sizeof...(r))
    {
        // handle the tail
        f(r...);
    }
}
*/

/*
template <class T, class... Args>
enable_if_t<is_constructible_v<T, Args...>, unique_ptr<T>> make_unique(
    Args&&... args)
{
    return unique_ptr<T>(new T(forward<Args>(args)...));
}
template <class T, class... Args>
enable_if_t<!is_constructible_v<T, Args...>, unique_ptr<T>> make_unique(
    Args&&... args)
{
    return unique_ptr<T>(new T{forward<Args>(args)...});
}

// With static if:
template <class T, class... Args>
unique_ptr<T> make_unique(Args&&... args)
{
    static_if(is_constructible_v<T, Args...>)
    {
        return unique_ptr<T>(new T(forward<Args>(args)...));
    }
    else
    {
        return unique_ptr<T>(new T{forward<Args>(args)...});
    }
}
*/

int main()
{
    return 0;
}