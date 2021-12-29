#ifndef TEST_REFLECTION_H
#define TEST_REFLECTION_H

#include "3rd/visit_struct/visit_struct.hpp"
#include <assert.h>
#include <string>
#include <vector>

struct test_struct_one {
    int a;
    float b;
    std::string c;
};

VISITABLE_STRUCT(test_struct_one, a, b, c);

struct debug_printer {
    template <typename T>
    void operator()(const char* name, const T& t) const
    {
        std::cout << "  " << name << ": " << t << std::endl;
    }
};

template <typename T>
void debug_print(const T& t)
{
    std::cout << "{\n";
    visit_struct::apply_visitor(debug_printer {}, t);
    std::cout << "}" << std::endl;
}

using spair = std::pair<std::string, std::string>;

struct test_visitor_one {
    std::vector<spair> result;

    void operator()(const char* name, const std::string& s)
    {
        result.emplace_back(spair { std::string { name }, s });
    }

    template <typename T>
    void operator()(const char* name, const T& t)
    {
        result.emplace_back(spair { std::string { name }, std::to_string(t) });
    }
};

void test_reflection()
{
    test_struct_one s { 5, 7.5f, "asdf" };
    debug_print(s);

    assert(visit_struct::field_count(s) == 3);
    assert(visit_struct::get<0>(s) == 5);
    assert(visit_struct::get<1>(s) == 7.5f);
    assert(visit_struct::get<2>(s) == "asdf");
    assert(visit_struct::get_name<0>(s) == std::string { "a" });
    assert(visit_struct::get_name<1>(s) == std::string { "b" });
    assert(visit_struct::get_name<2>(s) == std::string { "c" });
    assert(visit_struct::get_accessor<0>(s)(s) == visit_struct::get<0>(s));
    assert(visit_struct::get_accessor<1>(s)(s) == visit_struct::get<1>(s));
    assert(visit_struct::get_accessor<2>(s)(s) == visit_struct::get<2>(s));

    test_visitor_one vis1;
    visit_struct::apply_visitor(vis1, s);

    assert(vis1.result.size() == 3);
    assert(vis1.result[0].first == "a");
    assert(vis1.result[0].second == "5");
    assert(vis1.result[1].first == "b");
    assert(vis1.result[1].second == "7.500000");
    assert(vis1.result[2].first == "c");
    assert(vis1.result[2].second == "asdf");
}
#endif
