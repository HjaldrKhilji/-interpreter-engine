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
        using List_t=std::array<std::meta::info, sizeof...(Types)>;
        List_t reflection;
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
        },
        reflection{
            std::apply([](auto&... args) {
                return {^^args...};
            }, data)
        }
        {}
        constexpr value_type operator[](size_t index) {
            return book_keeper[index];
        }
    };
};

namespace runtime{
    template<typename... Ts>
    class Dynamic_unit{
        using List_t=std::array<std::meta::info, sizeof...(Ts)>;
        void *ptr;
        enum class Single_tag_type
        {
            empty________________________ //making sure any internal buffer is big enough
        };
        enum class Dual_Tag_type
        {
            empty________________________ //making sure any internal buffer is big enough
        };
        Single_tag_type tag;
        consteval std::string construct_name(std::string type_name_1, std::string type_name_2){
            return type_name_1+type_name_2;
        }
        consteval List_t construct_dual_type_list(auto dummy){
            List_t members;
            auto types= std::to_array({^^Ts...});
            auto name_of_dummy =std::meta::identifier_of(dummy);
            for(auto t_it=types.begin(); t_it<types.end(); t_it++) //t_it=test_iterator
            {
                auto name_of_member= std::meta::identifier_of(*t_it);
                for(auto n_t_it=t_it; n_t_it<types.end(); n_t_it++){
                    if(n_t_it!=t_it){
                        const_cast<std::string_view>(name_of_dummy)=construct_name(name_of_member, std::meta::identifier_of(n_t_it));
                        members.push_back(dummy);
                    }
                }
            }
            return members;
        }
        consteval List_t construct_single_type_list(auto dummy){
            List_t members;
            for (auto Type: {^^Ts...})
            {
                auto name_of_member= std::meta::identifier_of(Type);
                auto name_of_dummy =std::meta::identifier_of(dummy);
                const_cast<std::string_view>(name_of_dummy)=name_of_member;
                members.push_back(dummy);
            }
            return members;
        }
        consteval void construct_dual_tag(auto dummy){
            std::meta::define_aggregate(^^Dual_Tag_type, construct_dual_type_list(dummy));
        }
        consteval void construct_single_tag(auto dummy){
            std::meta::define_aggregate(^^Single_tag_type, construct_single_type_list(dummy));
            
        }
        consteval
        {
            construct_dual_tag(^^Dual_Tag_type::empty);
            construct_single_tag(^^Single_tag_type::empty);
        }
        void do_with_value(Dynamic_unit lhs){
            switch(tag){
                
            }
        }
    };
    
    template<typename... Ts>
    struct Runtime_storage_t{
        std::vector<Dynamic_unit<Ts...>> all_storage;
    };
    
};


int main()
{
    
    return 0;
}
