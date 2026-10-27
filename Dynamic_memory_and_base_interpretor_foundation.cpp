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
    using Delay = F<N, Args...>;//credits to bjarne stroustrup's c++ book from
    //2013 for this one
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
    };
    template<typename Head, typename... Types>
    struct get_variant_of_unique_ts<0, Head, Types...>
    {
        using Variant_t = std::variant<Head*, Types*...>;
    };
    template<typename ... Types>
    struct runtime_indexed_tuples {
        std::tuple<Types...> data;
        using value_type = typename get_variant_of_unique_ts<sizeof...(Types)
        - 1, Types...>::Variant_t;
        using Book_keeping_t = std::array<value_type, (sizeof...(Types))>;
        Book_keeping_t book_keeper;
        template<typename... Types_>
        constexpr runtime_indexed_tuples(Types_&&... b) :
        data{ std::make_tuple(std::forward<Types_>(b)...) },
        book_keeper{
            std::apply([](auto&... args) {
                return Book_keeping_t{args...};
            }, data)
        }
        {}
        constexpr value_type operator[](size_t index) {
            return book_keeper[index];
        }
        std::vector<std::meta::info> members;
        consteval
        {
            template for (std::size_t i=0;auto Type: {^^Ts...})
            {
                auto name_of_member=std::format("{}_{}", std::meta::identifier_of(^^Type), i);
                auto member_to_push= std::meta::data_member_options{.name = name_of_member}; //i is to impose uniqueness and to make the name different than the name of the type.
                members.push_back(std::meta::data_member_spec(Type, member_to_push));
                i++;
            }
        }
    };
};
namespace runtime{
    template<typename... Ts>
    class Dynamic_unit{
        void *ptr;
        std::size_t size;
        enum class Tag_type
        {
            empty________________________ //making sure any internal buffer is big enough
        };
        consteval
        {
            auto dummy=^^Tag_type::empty;
            auto name_of_dummy=  std::meta::identifier_of(dummy);
            std::string name_of_dummy_temp= name_of_dummy;
            std::vector<std::meta::info> members;
            template for (auto Type: {^^Ts...})
            {
                auto name_of_member= std::meta::identifier_of(^^Type);
                const_cast<std::string_view>(name_of_dummy)=name_of_member;
                members.push_back(dummy);
            }
            const_cast<std::string_view>(name_of_dummy)=name_of_dummy_temp;
        }
    };
    struct Runtime_storage_t{
        std::vector<Dynamic_unit> all_storage;
    };
};


int main()
{

    return 0;
}
