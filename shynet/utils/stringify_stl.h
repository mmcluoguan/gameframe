/**
	 1. 支持所有stl类型容器，接口简单只有一个函数，业务方传入容器变量即可使用

	   ```
	   std::vector<int> vec({1,2,3});
	   std::string str = shynet::utils::stringify::stl(vec);
	   // "[1,2,3]"

	   std::map<int, int> m;
	   m[1] = 100; m[2] = 200; m[3] = 300;
	   str = shynet::utils::stringify::stl(m);
	   // "{1:100,2:200,3:300}"
	   ```

	2. 支持多维嵌套容器

	  ```
	  std::vector<std::vector<int> > vec = {{1,2,3}, {4,5,6}};
	  str = shynet::utils::stringify::stl(vec);
	  // "[[1,2,3],[4,5,6]]"
	  ```

	3. 支持容器元素为自定义类型

	  ```
	  struct SelfType {
		int a_;
		int b_;

		SelfType(int a, int b) : a_(a), b_(b) {  }
	  };

	  // @NOTICE 业务方自己实现自定义类型转换成string的函数
	  std::string stl_one_(const SelfType &st) {
		std::ostringstream oss;
		oss << "(" << st.a_ << "," << st.b_ << ")";
		return oss.str();
	  }

	  std::vector<SelfType> vec;
	  vec.push_back(SelfType(1,2));
	  vec.push_back(SelfType(3,4));
	  std::string str = shynet::utils::stringify::stl(vec);
	  // "[(1,2),(3,4)]"
	  ```

	 4. 支持自定义样式

	   ```
	   // 自定义样式举例
	   // shynet::utils::STL_STYLE_ONE_BEAUTY 是我提供的一个默认带缩进换行的样式，业务方可以自己构造一个
	   // stl_one_style 类型的变量实现更高定制化的样式
	   std::string str = shynet::utils::stringify::stl(vec, shynet::utils::STL_STYLE_ONE_BEAUTY);
	   // [
	   //   "1",
	   //   "2",
	   //   "3"
	   // ]

 *
 */
#ifndef SHYNET_UTILS_STL_H
#define SHYNET_UTILS_STL_H

#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace shynet {
namespace utils {
    namespace stringify {

        struct stl_one_style {
            std::string delimiter_whole_prefix_;
            std::string delimiter_item_prefix_;
            std::string delimiter_item_suffix_;
            std::string delimiter_item_sep_;
            std::string delimiter_whole_suffix_;

            stl_one_style(const std::string& delimiter_whole_prefix = "[",
                const std::string& delimiter_item_prefix = "",
                const std::string& delimiter_item_suffix = "",
                const std::string& delimiter_item_sep = ",",
                const std::string& delimiter_whole_suffix = "]")
                : delimiter_whole_prefix_(delimiter_whole_prefix)
                , delimiter_item_prefix_(delimiter_item_prefix)
                , delimiter_item_suffix_(delimiter_item_suffix)
                , delimiter_item_sep_(delimiter_item_sep)
                , delimiter_whole_suffix_(delimiter_whole_suffix)
            {
            }
        };

        struct stl_two_style {
            std::string delimiter_whole_prefix_;
            std::string delimiter_key_prefix_;
            std::string delimiter_key_suffix_;
            std::string delimiter_kv_sep_;
            std::string delimiter_value_prefix_;
            std::string delimiter_value_suffix_;
            std::string delimiter_item_sep_;
            std::string delimiter_whole_suffix_;

            stl_two_style(const std::string& delimiter_whole_prefix = "{",
                const std::string& delimiter_key_prefix = "",
                const std::string& delimiter_key_suffix = "",
                const std::string& delimiter_kv_sep = ":",
                const std::string& delimiter_value_prefix = "",
                const std::string& delimiter_value_suffix = "",
                const std::string& delimiter_item_sep = ",",
                const std::string& delimiter_whole_suffix = "}")
                : delimiter_whole_prefix_(delimiter_whole_prefix)
                , delimiter_key_prefix_(delimiter_key_prefix)
                , delimiter_key_suffix_(delimiter_key_suffix)
                , delimiter_kv_sep_(delimiter_kv_sep)
                , delimiter_value_prefix_(delimiter_value_prefix)
                , delimiter_value_suffix_(delimiter_value_suffix)
                , delimiter_item_sep_(delimiter_item_sep)
                , delimiter_whole_suffix_(delimiter_whole_suffix)
            {
            }
        };

        static const stl_one_style STL_STYLE_ONE_DEFAULT("[", "", "", ",", "]");
        static const stl_one_style STL_STYLE_ONE_BEAUTY("[\n", "  \"", "\"", ",\n", "\n]");

        static const stl_two_style STL_STYLE_TWO_DEFAULT("{", "", "", ":", "", "", ",", "}");
        static const stl_two_style STL_STYLE_TWO_BEAUTY("{\n", "  \"", "\"", ": ", "", "", ",\n", "\n}");

        std::string stl_one_(uint8_t val) { return std::to_string(val); }
        std::string stl_one_(uint16_t val) { return std::to_string(val); }
        std::string stl_one_(uint32_t val) { return std::to_string(val); }
        std::string stl_one_(uint64_t val) { return std::to_string(val); }
        std::string stl_one_(int8_t val) { return std::to_string(val); }
        std::string stl_one_(int16_t val) { return std::to_string(val); }
        std::string stl_one_(int32_t val) { return std::to_string(val); }
        std::string stl_one_(int64_t val) { return std::to_string(val); }
        std::string stl_one_(float val) { return std::to_string(val); }
        std::string stl_one_(double val) { return std::to_string(val); }
        std::string stl_one_(long double val) { return std::to_string(val); }
        std::string stl_one_(const std::string& s) { return s; }
        template <typename T1, typename T2>
        std::string stl_one_(const std::pair<T1, T2>& p)
        {
            std::ostringstream ss;
            ss << "(" << p.first << "," << p.second << ")";
            return ss.str();
        }

        template <typename T>
        std::string stl_one_(const T& val, const stl_one_style& style = STL_STYLE_ONE_DEFAULT)
        {
            if (val.empty()) {
                return std::string();
            }

            std::ostringstream ss;
            ss << style.delimiter_whole_prefix_;
            typename T::const_iterator iter = val.begin();
            for (;;) {
                ss << style.delimiter_item_prefix_ << stl_one_(*iter++) << style.delimiter_item_suffix_;
                if (iter == val.end()) {
                    break;
                } else {
                    ss << style.delimiter_item_sep_;
                }
            }
            ss << style.delimiter_whole_suffix_;
            return ss.str();
        }

        template <typename T>
        std::string stl_two_(const T& val, const stl_two_style& style = STL_STYLE_TWO_DEFAULT)
        {
            if (val.empty()) {
                return std::string();
            }

            std::ostringstream ss;
            ss << style.delimiter_whole_prefix_;
            typename T::const_iterator iter = val.begin();
            for (;;) {
                ss << style.delimiter_key_prefix_ << stl_one_(iter->first) << style.delimiter_key_suffix_
                   << style.delimiter_kv_sep_
                   << style.delimiter_value_prefix_ << stl_one_(iter->second) << style.delimiter_value_suffix_;
                ++iter;
                if (iter == val.end()) {
                    break;
                } else {
                    ss << style.delimiter_item_sep_;
                }
            }
            ss << style.delimiter_whole_suffix_;
            return ss.str();
        }

        template <typename T, std::size_t N>
        std::string stl(const std::array<T, N>& val,
            const stl_one_style& style = STL_STYLE_ONE_DEFAULT)
        {
            return stl_one_<std::array<T, N>>(val, style);
        }

        template <typename T>
        std::string stl(const std::vector<T>& val,
            const stl_one_style& style = STL_STYLE_ONE_DEFAULT)
        {
            return stl_one_<std::vector<T>>(val, style);
        }

        template <typename T>
        std::string stl(const std::deque<T>& val,
            const stl_one_style& style = STL_STYLE_ONE_DEFAULT)
        {
            return stl_one_<std::deque<T>>(val, style);
        }

        template <typename T>
        std::string stl(const std::forward_list<T>& val,
            const stl_one_style& style = STL_STYLE_ONE_DEFAULT)
        {
            return stl_one_<std::forward_list<T>>(val, style);
        }

        template <typename T>
        std::string stl(const std::list<T>& val,
            const stl_one_style& style = STL_STYLE_ONE_DEFAULT)
        {
            return stl_one_<std::list<T>>(val, style);
        }

        template <typename T>
        std::string stl(const std::set<T>& val,
            const stl_one_style& style = STL_STYLE_ONE_DEFAULT)
        {
            return stl_one_<std::set<T>>(val, style);
        }

        template <typename T>
        std::string stl(const std::multiset<T>& val,
            const stl_one_style& style = STL_STYLE_ONE_DEFAULT)
        {
            return stl_one_<std::multiset<T>>(val, style);
        }

        template <typename T>
        std::string stl(const std::unordered_set<T>& val,
            const stl_one_style& style = STL_STYLE_ONE_DEFAULT)
        {
            return stl_one_<std::unordered_set<T>>(val, style);
        }

        template <typename T>
        std::string stl(const std::unordered_multiset<T>& val,
            const stl_one_style& style = STL_STYLE_ONE_DEFAULT)
        {
            return stl_one_<std::unordered_multiset<T>>(val, style);
        }

        template <typename K, typename T>
        std::string stl(const std::map<K, T>& val,
            const stl_two_style& style = STL_STYLE_TWO_DEFAULT)
        {
            return stl_two_<std::map<K, T>>(val, style);
        }

        template <typename K, typename T>
        std::string stl(const std::multimap<K, T>& val,
            const stl_two_style& style = STL_STYLE_TWO_DEFAULT)
        {
            return stl_two_<std::multimap<K, T>>(val, style);
        }

        template <typename K, typename T>
        std::string stl(const std::unordered_map<K, T>& val,
            const stl_two_style& style = STL_STYLE_TWO_DEFAULT)
        {
            return stl_two_<std::unordered_map<K, T>>(val, style);
        }

        template <typename K, typename T>
        std::string stl(const std::unordered_multimap<K, T>& val,
            const stl_two_style& style = STL_STYLE_TWO_DEFAULT)
        {
            return stl_two_<std::unordered_multimap<K, T>>(val, style);
        }

        // * deep copy noniterative containers e.g. stack,queue,priority_queue
        // * string begin with stack and priority_queue's top and queue's front

#define STL_NONITERATIVE(val, style, posfunc)                                                          \
    if (val.empty()) {                                                                                 \
        return std::string();                                                                          \
    }                                                                                                  \
    std::ostringstream ss;                                                                             \
    ss << style.delimiter_whole_prefix_;                                                               \
    for (;;) {                                                                                         \
        ss << style.delimiter_item_prefix_ << stl_one_(val.posfunc()) << style.delimiter_item_suffix_; \
        val.pop();                                                                                     \
        if (val.empty()) {                                                                             \
            break;                                                                                     \
        } else {                                                                                       \
            ss << style.delimiter_item_sep_;                                                           \
        }                                                                                              \
    }                                                                                                  \
    ss << style.delimiter_whole_suffix_;                                                               \
    return ss.str();

        template <typename T>
        std::string stl(std::stack<T> val,
            const stl_one_style& style = STL_STYLE_ONE_DEFAULT)
        {
            STL_NONITERATIVE(val, style, top);
        }

        template <typename T>
        std::string stl(std::queue<T> val,
            const stl_one_style& style = STL_STYLE_ONE_DEFAULT)
        {
            STL_NONITERATIVE(val, style, front);
        }

        template <typename T>
        std::string stl(std::priority_queue<T> val,
            const stl_one_style& style = STL_STYLE_ONE_DEFAULT)
        {
            STL_NONITERATIVE(val, style, top);
        }
    }
};
}

#endif
