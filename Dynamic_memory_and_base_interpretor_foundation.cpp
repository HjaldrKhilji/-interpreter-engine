#include <meta>
#include <format>
#include <type_traits>
#include <variant>
#include <functional>
#include <array>
#include <utility>
#include <algorithm>
#include <flat_map>
#include <ranges>
#include <tuple>
//revision of https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2026/p4198r0.pdf with reflections and other C++26 features
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
    };
    template<typename Head, typename... Types>
    struct get_variant_of_unique_ts<0, Head, Types...>
    {
        using Variant_t = std::variant<Head*, Types*...>;
    };
    template<template <typename...> typename dummy, typename ...T>
    constexpr std::array<std::meta::info, sizeof...(T)> get_reflections(dummy<T...>){
        std::tuple<T...> dummy_tuple;
        std::array<std::meta::info, std::variant_size_v<dummy<T...>>> type_reflections{std::apply([](auto&... args) {return {^^decltype(args)...};}, dummy_tuple)};
        return type_reflections;
    }
    template<typename T, typename T_2>
    concept is_super_set= requires (T_2& a){T{a};};
    template<typename T, typename T_2>
    requires is_super_set<T, T_2>
    constexpr T construct_if_super_set(T_2& operand){return operand;}
    template<typename T, typename T_2>
    requires (!is_super_set<T, T_2>)
    constexpr T construct_if_super_set(T_2& operand){return T{}; }
    template <template<typename...> typename tuple_like, typename...T>
    std::tuple<T...> get_empty_tuple(tuple_like<T...>){
        return std::tuple<T...>{};
    }
    template <typename... Types>
    struct type_folder;
    template<typename... Types>
    struct type_folder<std::tuple<Types...>>{
        using Variant_t=get_variant_of_unique_ts<sizeof...(Types)
        - 1, Types...>::Variant_t;
    };
    template<typename tuple_specialization, typename dynamic_t=type_folder<tuple_specialization>::Variant_t, template<class Key_t,class T,class compare = std::less<Key_t>,class Key_cont_t = std::vector<Key_t>,class Internal_map_t = std::vector<T>> class Mapped_t= std::flat_map>
    requires requires(tuple_specialization a){
        std::tuple{a};
    }
    struct runtime_indexed_tuples {
        tuple_specialization data;
        using value_type = type_folder<tuple_specialization>::Variant_t;//dosent include duplicates
        using dynamic_value_t= dynamic_t;
        constexpr static  inline std::array<std::meta::info, std::variant_size_v<dynamic_value_t>> type_reflections{get_reflections(std::declval<dynamic_value_t>())};
        constexpr static std::size_t number_of_tuple_types=std::tuple_size_v<tuple_specialization>;
        using Book_keeping_t = std::array<value_type, number_of_tuple_types>;
        using Book_keeping_map_t= Mapped_t<std::string, std::size_t, std::less<std::size_t>, std::array<std::string, number_of_tuple_types>, std::array<std::size_t, number_of_tuple_types> >;
        Book_keeping_t book_keeper;
        Book_keeping_map_t book_keeper_map;
        std::vector<dynamic_value_t> dynamic_book_keeping{};
        Mapped_t<std::string, std::size_t> dynamic_book_keeper_map{};
        using dynamic_storage_type=std::vector<dynamic_value_t>;
        constexpr runtime_indexed_tuples()=default;
        constexpr runtime_indexed_tuples(tuple_specialization&& src_tuple, Book_keeping_map_t&& map) :
        data{std::forward<tuple_specialization>(src_tuple)},
        book_keeper{
            std::apply([](auto&... args) {
                return Book_keeping_t{value_type(&args)...};
            }, data)
        },
        dynamic_book_keeping{
            construct_if_super_set<std::vector<dynamic_value_t>, Book_keeping_t>(book_keeper)
        },
        book_keeper_map{
            map
        },
        dynamic_book_keeper_map{std::from_range, book_keeper_map}
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
        constexpr dynamic_value_t get_dynamic_at_index(std::size_t index) {
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
            template for( std::size_t index=0; auto elem: type_reflections){
                if(index==type_index){
                    auto object=new [: type_reflections.at(index) :](input_stream);
                    dynamic_book_keeping.push_back(dynamic_value_t{object});
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
                    delete arg;//you may delete an eleemnt of the tuple (causing undefind behaviour since elements of tuples arent dynamically allocated) if the user dosent do proper range checks
                },
                dynamic_book_keeping.at(obj_index)
            );
            dynamic_book_keeping.pop_back();
        }
        constexpr void remove_obj_checked(std::size_t obj_index){ // returns false if not owned
            if constexpr(is_super_set<dynamic_value_t, value_type>){
                if(!obj_index>=std::tuple_size_v<decltype(data)>){
                    remove_obj(obj_index);
                }
                else{
                    throw std::string{"object not owned"};
                }
            }
        }
    };
};




int main()
{
    storage::runtime_indexed_tuples obj{std::tuple<int, double>{1, 2}, {}};
    return 0;
}
