#include <meta>
/*runtime_indexed_tuple from https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2026/p4198r0.pdf will be used here to provide fixed size heterogenous lists*/
template<typename... Ts>
class dynamic_unit{
    void *ptr;
    std::size_t size;
    enum class Tag_type
    {
        empty________________________//making sure any internal buffer is big enough,
        consteval
        {
                auto dummy=^^empty;
                auto name_of_dummy=  std::meta::identifier_of(dummy);
                std::string name_of_dummy_temp= name_of_dummy;
                template for (auto Type: {^^Ts...})
                {
                    auto name_of_member= std::meta::identifier_of(^^Type);
                    const_cast<std::string_view>(name_of_dummy)=name_of_member;
                    members.push(dummy);
                    i++;
                }
                const_cast<std::string_view>(name_of_dummy)=name_of_dummy_temp;

        }
    }
};

template<typename... Ts>
struct Storage_facility{
    std::vector<std::meta::info> members;//built in storage (supposed to be fast once C++ reflection magic start being optimized). It basically is the code tuples code, but except element is named and the names are so that the access wont require if statements.
    consteval
    {
        template for (std::size_t i=0;auto Type: {^^Ts...})
        {
            auto name_of_member=std::format("{}_{}", std::meta::identifier_of(^^Type), i);
            auto member_to_push;= std::meta::data_member_options{.name = name_of_member}; //i is to impose uniqueness and to make the name different than the name of the type.
            members.push(std::meta::data_member_spec(Type, name));
            i++;
        }
        std::meta::define_aggregate(^^Storage, member_specs);
    }
    template<typename T>
    T& get_value(){
        //todo
    }

};

int main()
{
    return 0;
}
