// Copyright (c) 2015 Vittorio Romeo
// License: AFL 3.0 | https://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#include <utility>
#include <iostream>
#include <type_traits>
#include <vrm/core/static_if.hpp>
#include <vrm/core/for_args.hpp>

// `static_if` is particularly useful when combined with other similar
// constructs, such as `for_args`.

// TODO: better example
// TODO: `for_args` description.

struct base_int_token
{
};
struct base_char_token
{
};

template <int TI>
struct int_token : std::integral_constant<int, TI>, base_int_token
{
};

template <char TC>
struct char_token : std::integral_constant<char, TC>, base_char_token
{
};

struct end_token
{
};



int main()
{
    using namespace vrm::core;

    for_args(
        [](auto tx)
        {
            static_if(std::is_base_of<base_int_token, decltype(tx)>{})
                .then([](auto ty)
                    {
                        static_if(bool_<ty % 2 == 0>{})
                            .then([](auto)
                                {
                                    std::cout << "even int token\n";
                                })
                            .else_([](auto)
                                {
                                    std::cout << "odd int token\n";
                                })(ty);
                    })
                .else_if(std::is_base_of<base_char_token, decltype(tx)>{})
                .then([](auto ty)
                    {
                        static_if(bool_<(ty >= 'a' && ty <= 'z')>{})
                            .then([](auto)
                                {
                                    std::cout << "lowercase char token\n";
                                })
                            .else_if(bool_<(ty >= 'A' && ty <= 'Z')>{})
                            .then([](auto)
                                {
                                    std::cout << "uppercase char token\n";
                                })
                            .else_([](auto)
                                {
                                    std::cout << "non-alpha char token\n";
                                })(ty);
                    })
                .else_([](auto)
                    {
                        std::cout << "unrecognized token\n";
                    })(tx);
        },
        int_token<2>{}, int_token<3>{}, char_token<'c'>{}, char_token<'C'>{});

    return 0;
}