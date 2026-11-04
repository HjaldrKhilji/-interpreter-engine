#include <meta>
#include <format>
#include <type_traits>
#include <variant>
#include <functional>
#include <array>
#include<utility>
#include <algorithm>
//runtime_indexed_tuples( https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2026/p4198r0.pdf )will be used here to provide fixed size heterogenous lists(for faster indexing), though as you will see, some aspects have been modified:
namespace look_up_namespace{
    template <typename Type>
    struct functions_set(){
        static inline constexpr std::vector<std::meta::info> all{std::meta::nonstatic_data_members_of(^^Type, std::meta::access_context::unchecked())};
        constexpr std::meta::info get_at_index(std::size_t index){
        auto result=all[index];
        static_assert( !( std::meta::is_public(result) && std::meta::is_function(result) ) ,"object, not valid function");
        return result;
        }
    };
};
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
    template<typename dynamic_t, typename ... Types>
    struct Runtime_engine {
        std::tuple<Types...> data;
        using type_folder=get_variant_of_unique_ts<sizeof...(Types)
        - 1, Types...>;//dosent include duplicates
        type_folder::List_t type_reflections;
        using value_type = type_folder::Variant_t;
        using Book_keeping_t = std::array<value_type, (sizeof...(Types))>;
        using Book_keeping_map_t= std::flat_map<std::string, value_type, std::less<value_type>, std::array<std::string>, std::array<value_type> >;
        Book_keeping_t book_keeper;
        Book_keeping_map_t book_keeper_map;
        std::vector<value_type> runtime_book_keeping{};
        std::flat_map<std::string, value_type> runtime_book_keeper_map{};
        using dynamic_storage_type=std::vector<dynamic_t>;
        dynamic_storage_type dynamic_set{};
        template<typename... Types_>
        constexpr Runtime_engine(Book_keeping_map_t& map, Types_&&... b) :
        data{ std::tuple(std::forward<Types_>(b)...) },
        book_keeper{
            std::apply([](dynamic_t, auto&... args) {
                return Book_keeping_t{args...};
            }, data)
        },
        type_reflections{
            type_folder::return_type_list()
        },
        book_keeper_map{
            map
        }
        {
            //dont want to rely on constructor order since bjarne stroustrup book says so
            std::insert_range(runtime_book_keeping.begin(), book_keeper);
            std::insert_range(runtime_book_keeper_map.begin(), book_keeper_map);
        }
        constexpr value_type operator[](size_t index) {
            return book_keeper[index];
        }
        void call_func(std::size_t obj_index, std::size_t func_index){
            std::visit(
                [](auto arg){
                            template for( std::size_t index=0; auto elem: std::meta::nonstatic_data_members_of(^^Type, std::meta::access_context::unchecked()) ){
                                if(index==func_index){
                                    auto ptr_to_mem_func= [:look_up_namespace::functions_set<decltype(arg)>::get_at_index(index):];
                                    (*arg.*ptr_to_mem_func)();
                                }
                                index++;
                            }
                },
                runtime_book_keeping.at(obj_index)
            );
        }
        void binary_op(std::size_t obj_index, std::size_t obj_2_index, std::size_t func_index){
            std::visit(
                [](auto arg, auto arg_2){
                    template for( std::size_t index=0; auto elem: std::meta::nonstatic_data_members_of(^^Type, std::meta::access_context::unchecked()) ){
                        if(index==func_index){
                            auto ptr_to_mem_func= [:look_up_namespace::functions_set<decltype(arg)>::get_at_index(index):];
                            (*arg.*ptr_to_mem_func)(*arg_2);
                        }
                        index++;
                    }
                },
                runtime_book_keeping.at(obj_index),
                runtime_book_keeping.at(obj_2_index)
            );
        }
        void add_new_obj(std::string init_string,std::size_t type_index, std::string key){
            template for( std::size_t index=0; auto elem: std::meta::nonstatic_data_members_of(^^Type, std::meta::access_context::unchecked()) ){
                if(index==type_index){
                    auto object=new [:type_reflections.at(index):](init_string);
                    runtime_book_keeping.push_back(value_type{object});
                    if(key!=""){
                        runtime_book_keeper_map[key]=runtime_book_keeping.back();
                    }
                }
                index++;
            }

        }
        void remove_obj(std::size_t obj_index){
            std::visit(
                [](auto arg){
                    delete arg;
                },
                runtime_book_keeping.at(obj_index)
            );
            runtime_book_keeping.pop_back();
        }



    };
};




int main()
{

    return 0;
}
