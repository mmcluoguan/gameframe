#ifndef SHYNET_UTILS_STRINGOP_H
#define SHYNET_UTILS_STRINGOP_H

#include "shynet/basic.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace shynet {
namespace utils {
    /*
		* 字符串简单操作
		*/
    namespace stringop {
        char* trim(char* str, std::function<int(int)> cb = isspace);
        std::string& trim(std::string& xstr, std::function<int(int)> cb = isspace);

        /// <summary>
        /// 拆分字符串
        /// </summary>
        /// <param name="target">目标</param>
        /// <param name="c">分割符</param>
        /// <param name="argv">拆分到的目标字符串数组</param>
        /// <param name="size">目标字符串数组大小</param>
        int split(char* target, const char* c, char** argv, int size);
        /**
			* 用`sep`分割`s`
			*
			* @param keep_empty_strings 分割后的字符串数组，如果有元素为空，是否保留这个元素
			* @param split_once 如果为true，那么最多分割一次
			*
			* @return 分割后的字符串数组
			*
			* @NOTICE
			*   if keep_empty_strings:
			*     '-a-b' -> ['', 'a', 'b']
			*     'a-b-' -> ['a', 'b', '']
			*     'a--b' -> ['a', '', 'b']
			*
			*
			*/
        std::vector<std::string> split(const std::string& s, char sep, bool keep_empty_strings = true, bool split_once = false);

        /**
			* 允许分隔符是字符串
			* like s='ab--cd' and sep='--' -> ['ab', 'cd']
			*
			*/
        std::vector<std::string> split(const std::string& s, const std::string& sep, bool keep_empty_strings = true);
        std::vector<std::string> split(const std::string& s, const char* sep, bool keep_empty_strings = true);

        /**
			* 可指定多个分隔符，一次到位~
			* like s='a-b*c' and sep='-*' -> ['a', 'b', 'c']
			*
			*/
        std::vector<std::string> split_any(const std::string& s, const std::string& charlist, bool keep_empty_strings = true);

        /**
			* 用 '\r' 或 '\n' 或 '\r\n' 分割
			*
			* @param keep_ends 如果为true，则把该行的结束符（'\r'或'\n'或'\r\n'）跟在该行后
			*
			*/
        std::vector<std::string> splitlines(const std::string& s, bool keep_ends = false);

        template <typename... Args>
        std::string str_format(const std::string& format, Args... args)
        {
            auto size_buf = std::snprintf(nullptr, 0, format.c_str(), std::forward<Args>(args)...) + 1;
            std::unique_ptr<char[]> buf(new char[size_buf]);

            if (!buf)
                return std::string("");

            std::snprintf(buf.get(), size_buf, format.c_str(), std::forward<Args>(args)...);
            return std::string(buf.get(), buf.get() + size_buf - 1);
        }

        std::string replace(const std::string& data, const std::string& toFind, const std::string& toReplace);

    };
}
}

#endif
