#include <meta>
#include <format>
#include <type_traits>
#include <variant>
#include <functional>
#include <array>
#include <utility>
#include <algorithm>
#include <flat_map>
//revision of https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2026/p4198r0.pdf )
namespace look_up_namespace{
    template <typename Type>
    struct functions_set{
        static inline constexpr std::vector<std::meta::info> all{std::meta::nonstatic_data_members_of(^^Type, std::meta::access_context::unchecked())};
        constexpr std::size_t get_index(std::string& name){
            template for(std::size_t index=0; auto elem: all){
                if(std::meta::identifier_of(elem)==name){
                    return index;
                }
                index++;
            }
            throw std::out_of_range{"out of bound compile time access to look_up_namespace::functions_set::all vector"};
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
        constexpr static std::size_t number_of_types=1+sizeof...(Types);
    };
    template<typename dynamic_t, typename ... Types>
    struct runtime_indexed_tuples {
        std::tuple<Types...> data;
        using type_folder=get_variant_of_unique_ts<sizeof...(Types)
        - 1, Types...>;//dosent include duplicates
        type_folder::List_t type_reflections;
        using value_type = type_folder::Variant_t;
        using Book_keeping_t = std::array<value_type, (sizeof...(Types))>;
        using Book_keeping_map_t= std::flat_map<std::string, std::size_t, std::less<std::size_t>, std::array<std::string, type_folder::number_of_types>, std::array<std::size_t, type_folder::number_of_types> >;
        Book_keeping_t book_keeper;
        Book_keeping_map_t book_keeper_map;
        std::vector<value_type> dynamic_book_keeping{};
        std::flat_map<std::string, std::size_t> dynamic_book_keeper_map{};
        using dynamic_storage_type=std::vector<dynamic_t>;
        dynamic_storage_type dynamic_set{};
        template<typename... Types_>
        constexpr runtime_indexed_tuples(Book_keeping_map_t& map, Types_&&... b) :
        data{ std::tuple(std::forward<Types_>(b)...) },
        book_keeper{
            std::apply([](dynamic_t, auto&... args) {
                return Book_keeping_t{args...};
            }, data)
        },
        dynamic_book_keeping{
            book_keeper
        },
        dynamic_book_keeper_map{
            book_keeper_map
        },
        type_reflections{
            type_folder::return_type_list()
        },
        book_keeper_map{
            map
        }
        {
        }

    private:
        template<typename T>
        void call_func_impl(T arg, std::size_t func_index){
            template for( std::size_t index=0; auto elem: std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::unchecked()) ){
                if(index==func_index){
                    auto ptr_to_mem_func= elem;
                    (arg.*ptr_to_mem_func)();
                    break;
                }
                index++;
            }
            throw std::out_of_range{"function index out of range in operation"};
        }
        template<typename T, typename T2>
        void call_func_impl(T arg, T2 arg_2, std::size_t func_index){
            template for( std::size_t index=0; auto elem: std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::unchecked()) ){
                if(index==func_index){
                    auto ptr_to_mem_func= elem;
                    (arg.*ptr_to_mem_func)(arg_2);
                    break;
                }
                index++;
            }
            throw std::out_of_range{"function index out of range in binary operation"};
        }
    public:
        constexpr value_type get_at_index(std::size_t index) {
            return book_keeper.at(index);
        }
        constexpr value_type get_dynamic_at_index(std::size_t index) {
            return dynamic_book_keeping.at(index);
        }
        constexpr std::size_t get_index_from_map(std::string& name){
            return book_keeper_map.at(name);
        }
        constexpr std::size_t get_dynamic_index_from_map(std::string& name){
            return dynamic_book_keeper_map.at(name);
        }
        constexpr void call_func(std::size_t obj_index, std::size_t func_index){
            std::visit(
                [func_index](auto arg){
                    call_func_impl(*arg, func_index);
                },
                dynamic_book_keeping.at(obj_index)
            );
        }

        constexpr void binary_op(std::size_t obj_index, std::size_t obj_2_index, std::size_t func_index){
            std::visit(
                [func_index](auto arg, auto arg_2){
                    call_func_impl(*arg, *arg_2, func_index);
                },
                dynamic_book_keeping.at(obj_index),
                       dynamic_book_keeping.at(obj_2_index)
            );
        }
        constexpr void call_func(std::size_t obj_index, std::string& func_index){
            std::visit(
                [func_index](auto arg){
                    call_func_impl(*arg, look_up_namespace::functions_set<decltype(arg)>::get_index(func_index));
                },
                dynamic_book_keeping.at(obj_index)
            );
        }

        constexpr void binary_op(std::size_t obj_index, std::size_t obj_2_index, std::string& func_index){
            std::visit(
                [func_index](auto arg, auto arg_2){
                    call_func_impl(*arg, *arg_2, look_up_namespace::functions_set<decltype(arg)>::get_index(func_index));
                },
                dynamic_book_keeping.at(obj_index),
                       dynamic_book_keeping.at(obj_2_index)
            );
        }
        constexpr void add_new_obj(std::iostream& input_stream,std::size_t type_index, std::string& key){
            template for( std::size_t index=0; auto elem: std::meta::nonstatic_data_members_of(^^Type, std::meta::access_context::unchecked()) ){
                if(index==type_index){
                    auto object=new [:type_reflections.at(index):](input_stream);
                    dynamic_book_keeping.push_back(value_type{object});
                    if(key!=""){
                        dynamic_book_keeper_map[key]=dynamic_book_keeping.size()-1;
                    }
                    break;
                }
                index++;
            }

        }
        constexpr void remove_obj(std::size_t obj_index){
            std::visit(
                [](auto arg){
                    delete arg;
                },
                dynamic_book_keeping.at(obj_index)
            );
            dynamic_book_keeping.pop_back();
        }



    };
};




int main()
{

    return 0;
}
