#pragma once
#include <functional>

namespace shynet
{
	namespace utils {
        namespace {

            /// everyone
            template <typename T>
            inline void hash_combine(size_t& seed, const T& val) {
                seed ^= std::hash<T>()(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }

            /// last
            template <typename T>
            inline void hash_val(size_t& seed, const T& val) {
                hash_combine(seed, val);
            }

            /// recursive
            template <typename T, typename... Types>
            inline void hash_val(size_t& seed, const T& val, const Types&... args) {
                hash_combine(seed, val);
                hash_val(seed, args...);
            }

        }

        /*
        * 利用变参模板和std::hash实现的万能哈希
        *   可通过多种类型的多个变量组合生成哈希值，使得哈希结果更均匀
        */
        template <typename... Types>
        inline size_t hash_val(const Types&... args) {
            size_t seed = 0;
            inner::hash_val(seed, args...);
            return seed;
        }
	}
}