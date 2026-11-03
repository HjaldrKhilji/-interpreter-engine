#include <meta>
#include <format>
#include <type_traits>
#include <variant>
#include <functional>
#include <array>
#include<utility>
//runtime_indexed_tuples( https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2026/p4198r0.pdf )will be used here to provide fixed size heterogenous lists(for faster indexing), though as you will see, some aspects have been modified:
namespace storage{
    template<template<std::size_t N, typename...> class F, std::size_t N,
    typename ... Args>
    using Delay = F<N, Args...>;
    template< std::size_t N, typename Head, typename ...Types>
    struct get_variant_of_unique_ts :
    std::conditional_t<
    (std::is_same_v<Head, Types> || ...),
    Delay<get_variant_of_unique_ts, N - 1, Types... >,
    Delay<get_variant_of_unique_ts, N - 1, Head, Types... >
    >
    {
        using intermediate =
        std::conditional_t< (std::is_same_v<Head, Types> || ...),
        Delay<get_variant_of_unique_ts, N - 1, Types... >,
        Delay<get_variant_of_unique_ts, N - 1, Head, Types... >
        >;
        using Variant_t = typename intermediate::Variant_t;
        using List_t    = typename intermediate::List_t;
        using intermediate::return_type_list;
    };
    template<typename Head, typename... Types>
    struct get_variant_of_unique_ts<0, Head, Types...>
    {
        using Variant_t = std::variant<Head*, Types*...>;
        using List_t=std::array<std::meta::info, 1+sizeof...(Types)>;
        static constexpr List_t return_type_list(){
            return List_t{^^Head, ^^Types...};
        }
    };
    template<typename ... Types>
    struct runtime_indexed_tuples {
        std::tuple<Types...> data;
        using type_folder=get_variant_of_unique_ts<sizeof...(Types)
        - 1, Types...>;
        type_folder::List_t type_reflections;//dosent include duplicates
        using value_type = type_folder::Variant_t;
        using Book_keeping_t = std::array<value_type, (sizeof...(Types))>;
        Book_keeping_t book_keeper;
        template<typename... Types_>
        constexpr runtime_indexed_tuples(Types_&&... b) :
        data{ std::tuple(std::forward<Types_>(b)...) },
        book_keeper{
            std::apply([](auto&... args) {
                return Book_keeping_t{args...};
            }, data)
        },
        type_reflections{
            type_folder::return_type_list()
        }
        {}
        constexpr value_type operator[](size_t index) {
            return book_keeper[index];
        }
    };
};




int main()
{

    return 0;
}
