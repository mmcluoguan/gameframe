#include "frmpub/basic.h"
#include "shynet/basic.h"
#include "shynet/utils/stuff.h"
#include <iostream>
#include <regex>
#include <stdexcept>

typedef std::string String;
using namespace std::literals::string_literals;

class Strings {
public:
    static String TrimStart(const std::string& data)
    {
        String s = data;
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        return s;
    }

    static String TrimEnd(const std::string& data)
    {
        String s = data;
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(),
            s.end());
        return s;
    }

    static String Trim(const std::string& data)
    {
        return TrimEnd(TrimStart(data));
    }

    static String Replace(const String& data, const String& toFind, const String& toReplace)
    {
        String result = data;
        size_t pos = 0;
        while ((pos = result.find(toFind, pos)) != String::npos) {
            result.replace(pos, toFind.length(), toReplace);
            pos += toReplace.length();
            pos = result.find(toFind, pos);
        }
        return result;
    }
};

static String Nameof(const String& name)
{
    std::smatch groups;
    String str = Strings::Trim(name);
    if (std::regex_match(str, groups, std::regex(u8R"(^&?([_a-zA-Z]\w*(->|\.|::))*([_a-zA-Z]\w*)$)"))) {
        if (groups.size() == 4) {
            return groups[3];
        }
    }
    throw std::invalid_argument(Strings::Replace(u8R"(nameof(#). Invalid identifier "#".)", u8"#", name));
}

#define nameof(name) Nameof(u8## #name##s)
#define cnameof(name) Nameof(u8## #name##s).c_str()

enum TokenType {
    COMMA,
    PERIOD,
    Q_MARK
};

struct MyClass {
    enum class MyEnum : char {
        AAA = -8,
        BBB = '8',
        CCC = AAA + BBB
    };
};

int main()
{
    frmpub::JosnMsgId msgid = frmpub::JosnMsgId::ADMIN_WORLD_BEGIN;
    String greetings = u8"Hello"s;
    std::cout << nameof(COMMA) << std::endl;
    std::cout << nameof(TokenType::PERIOD) << std::endl;
    std::cout << nameof(TokenType::Q_MARK) << std::endl;
    std::cout << nameof(int) << std::endl;
    std::cout << nameof(std::string) << std::endl;
    std::cout << nameof(Strings) << std::endl;
    std::cout << nameof(String) << std::endl;
    std::cout << nameof(greetings) << std::endl;
    std::cout << nameof(&greetings) << std::endl;
    std::cout << nameof(greetings.c_str) << std::endl;
    std::cout << nameof(std::string::npos) << std::endl;
    std::cout << nameof(MyClass::MyEnum::AAA) << std::endl;
    std::cout << nameof(MyClass::MyEnum::BBB) << std::endl;
    std::cout << nameof(MyClass::MyEnum::CCC) << std::endl;

    std::cin.get();
    return 0;
}
