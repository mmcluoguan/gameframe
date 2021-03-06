#include "shynet/utils/stringop.h"
#include <algorithm>
#include <arpa/inet.h>
#include <cctype>
#include <cstring>
#include <fcntl.h>
#include <sstream>
#include <sys/resource.h>
#include <unistd.h>

namespace shynet {
namespace utils {
    namespace stringop {

        std::string& trim(std::string& xstr, std::function<int(int)> cb)
        {
            if (xstr.empty()) {
                return xstr;
            }

            std::string::iterator c = xstr.begin();
            for (; c != xstr.end() && cb(*c++);)
                ;
            xstr.erase(xstr.begin(), --c);

            for (c = xstr.end(); c != xstr.begin() && cb(*--c);)
                ;
            xstr.erase(++c, xstr.end());

            return xstr;
        }

        char* trim(char* str, std::function<int(int)> cb)
        {
            char* p = str;
            char* p1;
            if (p) {
                p1 = p + strlen(str) - 1;
                while (*p && cb(*p))
                    p++;
                while (p1 > p && cb(*p1))
                    *p1-- = '\0';
            }
            return p;
        }

        int split(char* target, const char* c, char** argv, int size)
        {
            int i = 0;
            char* p = NULL;
            char* tp;
            p = strtok_r(target, c, &tp);
            while (p && i < size) {
                argv[i] = p;
                ++i;
                p = strtok_r(NULL, c, &tp);
            }
            return i;
        }

        std::vector<std::string> split(const std::string& s, char sep, bool keep_empty_strings, bool split_once)
        {
            if (s.empty()) {
                return std::vector<std::string>();
            }

            std::vector<std::string> ret;

            if (split_once) {
                std::size_t pos = s.find(sep);
                if (pos == std::string::npos) {
                    ret.push_back(s);
                    return ret;
                } else if (pos == 0) {
                    if (keep_empty_strings) {
                        ret.push_back(std::string());
                    }

                    ret.push_back(s.substr(1));
                    return ret;
                } else if (pos == (s.length() - 1)) {
                    ret.push_back(s.substr(0, s.length() - 1));
                    if (keep_empty_strings) {
                        ret.push_back(std::string());
                    }

                    return ret;
                }

                ret.push_back(s.substr(0, pos));
                ret.push_back(s.substr(pos + 1));
                return ret;
            }

            std::stringstream ss(s);
            std::string item;
            for (; std::getline(ss, item, sep);) {
                if (!keep_empty_strings && item.empty()) {
                    continue;
                }

                ret.push_back(item);
            }

            /// ???????????????????????????????????????????????????????????????append?????????item??????
            /// std::getline???????????????append????????????????????????????????????~
            if (keep_empty_strings && *s.rbegin() == sep) {
                ret.push_back(std::string());
            }

            return ret;
        }
        std::vector<std::string> split(const std::string& s, const std::string& sep, bool keep_empty_strings)
        {
            std::vector<std::string> ret;
            if (s.empty()) {
                return ret;
            }
            if (sep.empty()) {
                ret.push_back(s);
                return ret;
            }

            const std::size_t s_len = s.length();
            const std::size_t sep_len = sep.length();
            std::size_t l = 0;
            std::size_t r = 0;
            for (;;) {
                r = s.find(sep, l);
                if (r != std::string::npos) {
                    if (l != r || keep_empty_strings) {
                        ret.push_back(s.substr(l, r - l));
                    }
                    l = r + sep_len;
                } else {
                    if (l < s_len || keep_empty_strings) {
                        ret.push_back(s.substr(l, r - l));
                    }
                    break;
                }
            }
            return ret;
        }
        std::vector<std::string> split(const std::string& s, const char* sep, bool keep_empty_strings)
        {
            return split(s, std::string(sep), keep_empty_strings);
        }
        std::vector<std::string> split_any(const std::string& s, const std::string& charlist, bool keep_empty_strings)
        {
            std::vector<std::string> ret;
            if (s.empty()) {
                return ret;
            }
            if (charlist.empty()) {
                ret.push_back(s);
                return ret;
            }

            const std::size_t s_len = s.length();
            std::size_t l = 0;
            std::size_t r = 0;
            for (;;) {
                r = s.find_first_of(charlist, l);
                if (r != std::string::npos) {
                    if (l != r || keep_empty_strings) {
                        ret.push_back(s.substr(l, r - l));
                    }
                    l = r + 1;
                } else {
                    if (l < s_len || keep_empty_strings) {
                        ret.push_back(s.substr(l, r - l));
                    }
                    break;
                }
            }
            return ret;
        }
        std::vector<std::string> splitlines(const std::string& s, bool keep_ends)
        {
            std::vector<std::string> ret;
            if (s.empty()) {
                return ret;
            }
            std::size_t len = s.length();
            std::size_t l = 0;
            std::size_t r = 0;
            std::string item;
            for (; r != len; r++) {
                if (s[r] == '\r') {
                    item = s.substr(l, r - l);
                    if (keep_ends) {
                        item += '\r';
                    }
                    if ((r + 1) != len && s[r + 1] == '\n') {
                        r++;
                        if (keep_ends) {
                            item += '\n';
                        }
                    }
                    l = r + 1;
                    ret.push_back(item);
                } else if (s[r] == '\n') {
                    item = s.substr(l, r - l);
                    if (keep_ends) {
                        item += '\n';
                    }
                    l = r + 1;
                    ret.push_back(item);
                }
            }
            if (l < r) {
                ret.push_back(s.substr(l, r - l));
            }
            return ret;
        }

        std::string replace(const std::string& data, const std::string& toFind, const std::string& toReplace)
        {
            std::string result = data;
            size_t pos = 0;
            while ((pos = result.find(toFind, pos)) != std::string::npos) {
                result.replace(pos, toFind.length(), toReplace);
                pos += toReplace.length();
                pos = result.find(toFind, pos);
            }
            return result;
        }

        /**
         * @brief ???????????????????????????
         * @param data ?????????
         * @return ?????????????????????
        */
        std::string toupper(std::string& data)
        {
            std::string dst;
            std::transform(data.begin(), data.end(), back_inserter(dst), ::toupper);
            return dst;
        }

        /**
         * @brief ???????????????????????????
         * @param data ?????????
         * @return ?????????????????????
        */
        std::string tolower(std::string& data)
        {
            std::string dst;
            std::transform(data.begin(), data.end(), std::back_inserter(dst), ::tolower);
            return dst;
        }
    }
}
}
