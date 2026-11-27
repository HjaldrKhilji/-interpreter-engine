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

#include<iostream>

#include<inplace_vector>

//revision of https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2026/p4198r0.pdf with reflections and other C++26 features
namespace look_up_namespace {
    template < typename Type >
    constexpr auto& Internal_content_data() {
        static constexpr auto index_data=std::meta::nonstatic_data_members_of(^^Type, std::meta::access_context::unchecked());
        return index_data;
    }
    template < typename Type >
    constexpr std::meta::info get_meta_obj(const std::size_t & index) {
        if consteval{
            return Internal_content_data<Type>().at(index);
        }
        else{
            //for each loop:
            std::size_t i=0;
            template for(auto& x: Internal_content_data<Type>()){
                if(i==index){
                    return x;
                }
                i++;
            }
        }
    }
    template < typename Type >
    constexpr std::meta::info get_meta_obj_string(const std::string & name) {
        //due to a compiler bug I couldnt make the variable below "static constexpr auto".
        auto look_up_data= []() consteval{
            std::flat_map<std::string, std::meta::info> result;
            std::ranges::for_each(std::meta::nonstatic_data_members_of(^^Type, std::meta::access_context::unchecked()), [&result](auto& elem){
                result[std::string{std::meta::identifier_of(elem)}]=elem;
            });
            return result;
        }();    
        if consteval{
            return look_up_data.at(name);
        }
        else{
            template for(auto& x: Internal_content_data<Type>()){
                if(std::meta::identifier_of(x)==name){
                    return x;
                }
            }
        }
    }
};
namespace storage {
    template < template < std::size_t N, typename... > class F, std::size_t N,
    typename...Args >
    using Delay = F < N, Args... > ;
    template < std::size_t N, typename Head, typename...Types >
    struct get_variant_of_unique_ts:
    std::conditional_t <
    (std::is_same_v < Head, Types > || ...),
    Delay < get_variant_of_unique_ts, N - 1, Types... > ,
    Delay < get_variant_of_unique_ts, N - 1, Head, Types... >
    >
    {
        using intermediate =
        std::conditional_t < (std::is_same_v < Head, Types > || ...),
        Delay < get_variant_of_unique_ts,
        N - 1,
        Types... > ,
        Delay < get_variant_of_unique_ts,
        N - 1,
        Head,
        Types... >
        >
        ;
        using Variant_t = typename intermediate::Variant_t;
    };
    template < typename Head, typename...Types >
    struct get_variant_of_unique_ts < 0, Head, Types... > {
        using Variant_t = std::variant < Head * ,
        Types * ... > ;
    };
    template < template < typename... > typename Dummy, typename...T >
    constexpr std::array < std::meta::info, sizeof...(T) > get_reflections(Dummy < T... > ) {
        std::tuple < T... > dummy_tuple;
        std::array < std::meta::info, std::variant_size_v < Dummy < T... >>> type_reflections {
            std::apply([](auto & ...args) {
                return {
                    ^^ decltype(args)...
                };
            }, dummy_tuple)
        };
        return type_reflections;
    }
    template <typename Result_t, typename... T>
    constexpr Result_t return_name_to_index_mapping(std::variant<T...> dummy){
        return []<std::size_t... indexes>(std::index_sequence<indexes...>){
            return
            Result_t{std::from_range,
                std::array{
                    std::pair{std::meta::display_string_of(^^T), indexes}...
                }
            };
        }.template operator()(std::make_index_sequence<sizeof...(T)>{});
    }
    template < typename T, typename T_2 >
    concept is_super_set = requires(T_2 & a) {
        T {
            a
        };
    };
    template < typename T, typename T_2 >
    requires is_super_set < T, T_2 >
    constexpr T construct_if_super_set(T_2 & operand) {
        return operand;
    }
    template < typename T, typename T_2 >
    requires(!is_super_set < T, T_2 > )
    constexpr T construct_if_super_set(T_2 & operand) {
        return T {};
    }
    template < template < typename... > typename tuple_like, typename...T >
    std::tuple < T... > get_empty_tuple(tuple_like < T... > ) {
        return std::tuple < T... > {};
    }
    template < typename...Types >
    struct type_folder;
    template < typename...Types >
    struct type_folder < std::tuple < Types... >> {
        using Variant_t = get_variant_of_unique_ts < sizeof...(Types) -
        1,
        Types... > ::Variant_t;
    };
    template < typename tuple_specialization, typename dynamic_t = type_folder < tuple_specialization > ::Variant_t, template < class Key_t, class T, class compare = std::less < Key_t > , class Key_cont_t = std::vector < Key_t > , class Internal_content_t = std::vector < T >> class Mapped_t = std::flat_map >
    requires requires(tuple_specialization a) {
        std::tuple {
            a
        };
    }
    struct runtime_indexed_tuples {
        tuple_specialization data;
        using value_type = type_folder < tuple_specialization > ::Variant_t; //dosent include duplicates
        using dynamic_value_t = dynamic_t;
        constexpr static std::size_t size_of_variant=std::variant_size_v < dynamic_value_t >;
        constexpr static inline std::array < std::meta::info, size_of_variant> type_reflections {
            get_reflections(std::declval < dynamic_value_t > ())
        };
        using Type_name_to_index_map_t= Mapped_t<
        std::string_view, std::size_t, std::less<std::string_view>,
        std::inplace_vector < std::string_view, size_of_variant >,
        std::inplace_vector < std::size_t, size_of_variant >
        >;
        constexpr static inline Type_name_to_index_map_t name_to_index_map{
            return_name_to_index_mapping<Type_name_to_index_map_t>(dynamic_value_t{})
        };
        constexpr static std::size_t number_of_tuple_types = std::tuple_size_v < tuple_specialization > ;
        using Book_keeping_t = std::array < value_type, number_of_tuple_types > ;
        using Book_keeping_map_t = Mapped_t < std::string, std::size_t, std::less < std::size_t > , std::array < std::string, number_of_tuple_types > , std::array < std::size_t, number_of_tuple_types > > ;
        Book_keeping_t book_keeper;
        Book_keeping_map_t book_keeper_map;
        std::vector < dynamic_value_t > dynamic_book_keeping {};
        Mapped_t < std::string, std::size_t > dynamic_book_keeper_map {};
        using dynamic_storage_type = std::vector < dynamic_value_t > ;
        constexpr runtime_indexed_tuples() =
        default;
        constexpr runtime_indexed_tuples(tuple_specialization && src_tuple, Book_keeping_map_t && map):
        data {
            std::forward < tuple_specialization > (src_tuple)
        },
        book_keeper {
            std::apply([](auto & ...args) {
                return Book_keeping_t {
                    value_type( & args)...
                };
            }, data)
        },
        dynamic_book_keeping {
            construct_if_super_set < std::vector < dynamic_value_t > , Book_keeping_t > (book_keeper)
        },
        book_keeper_map {
            map
        },
        dynamic_book_keeper_map {
            std::from_range, book_keeper_map
        } {
            
        }
        
        constexpr value_type get_at_index(std::size_t index) {
            return book_keeper.at(index);
        }
        constexpr dynamic_value_t get_dynamic_at_index(std::size_t index) {
            return dynamic_book_keeping.at(index);
        }
        constexpr std::size_t get_index_from_map(const std::string & name) {
            return book_keeper_map.at(name);
        }
        constexpr std::size_t get_dynamic_index_from_map(const std::string & name) {
            return dynamic_book_keeper_map.at(name);
        }
        constexpr static std::size_t get_type_index(const std::string name){
            return name_to_index_map.at(name);
        }
        constexpr void call_func(std::size_t obj_index, std::size_t func_index) {
            std::visit(
                [func_index](auto arg) {
                    (*arg.*[:look_up_namespace::get_meta_obj<decltype(*arg)>(func_index):])();
                },
                dynamic_book_keeping.at(obj_index)
            );
        }
        
        constexpr void binary_op(std::size_t obj_index, std::size_t obj_2_index, std::size_t func_index) {
            std::visit(
                [func_index](auto arg, auto arg_2) {
                    (*arg.*[:look_up_namespace::get_meta_obj<decltype(*arg)>(func_index):])(*arg_2);
                },
                dynamic_book_keeping.at(obj_index),
                       dynamic_book_keeping.at(obj_2_index)
            );
        }
        constexpr void call_func(std::size_t obj_index, const std::string & func_name) {
            std::visit(
                [&func_name](auto arg) {
                    (*arg.*[:look_up_namespace::get_meta_obj_string<decltype(*arg)>(func_name):])();
                },
                dynamic_book_keeping.at(obj_index)
            );
        }
        
        constexpr void binary_op(std::size_t obj_index, std::size_t obj_2_index, const std::string & func_name) {
            std::visit(
                [&func_name](auto arg, auto arg_2) {
                    (*arg.*[:look_up_namespace::get_meta_obj_string<decltype(*arg)>(func_name):])(*arg_2);
                },
                dynamic_book_keeping.at(obj_index),
                       dynamic_book_keeping.at(obj_2_index)
            );
        }
        
        template < typename T >
        constexpr void add_new_obj(T & input_stream, std::size_t type_index, const std::string & key) {
            template
            for (std::size_t index = 0; auto elem: type_reflections) {
                if (index == type_index) {
                    auto object = new [:type_reflections.at(index):](input_stream);
                    dynamic_book_keeping.push_back(dynamic_value_t {
                        object
                    });
                    dynamic_book_keeper_map[key] = dynamic_book_keeping.size() - 1; //using operator[] since value_type is a built in type (no copy constructor overhead), hence no point of using emplace here, or in a similiar place.
                    break;
                }
                index++;
            }
            
        }
        constexpr void dynamic_update_name_of_obj(const std::string & old_name, const std::string & name) {
            auto it = dynamic_book_keeper_map.find(old_name);
            auto value = it -> second;
            dynamic_book_keeper_map.erase(it);
            dynamic_book_keeper_map[name] = value;
        }
        constexpr void update_name_of_obj(const std::string & old_name,const  std::string & name) {
            auto it = book_keeper_map.find(old_name);
            auto value = it -> second;
            book_keeper_map.erase(it);
            book_keeper_map[name] = value;
        }
        constexpr void dynamic_remove_name_of_obj(const std::string & old_name,const  std::string & name) {
            auto it = dynamic_book_keeper_map.find(old_name);
            dynamic_book_keeper_map.erase(it);
        }
        constexpr void remove_name_of_obj(const std::string & old_name,const  std::string & name) {
            auto it = book_keeper_map.find(old_name);
            book_keeper_map.erase(it);
        }
        constexpr void remove_obj() {
            std::visit(
                [](auto arg) {
                    delete arg; //you may delete an eleemnt of the tuple (causing undefind behaviour since elements of tuples arent dynamically allocated) if the user dosent do proper range checks
                },
                dynamic_book_keeping.back()
            );
            dynamic_book_keeping.pop_back();
        }
        constexpr void remove_obj_checked() { // returns false if not owned
            if constexpr(is_super_set < dynamic_value_t, value_type > ) {
                if (!(dynamic_book_keeping.size() >= std::tuple_size_v < decltype(data) >) ) {
                    remove_obj();
                } else {
                    throw std::string {
                        "object not owned"
                    };
                }
            }
        }
    };
};
int main() {
    //testing each interface:
    storage::runtime_indexed_tuples obj {
        std::tuple < std::string, std::string > {
            "a1",
            "a2"
        }, {}
    };
    std::cout<<decltype(obj)::get_type_index("string");
    obj.call_func(0, std::string{"print"});
    return 0;
}
