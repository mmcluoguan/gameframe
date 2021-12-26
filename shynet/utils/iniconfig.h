#ifndef SHYNET_UTILS_INICONFIG_H
#define SHYNET_UTILS_INICONFIG_H

#include "shynet/basic.h"
#include "shynet/utils/stringop.h"
#include <sstream>
#include <unordered_map>

namespace shynet {
namespace utils {
    /**
     * @brief ini配置文件解析器
    */
    class IniConfig final : public Nocopy {
        /**
         * @brief 节点
        */
        struct node {
            std::string section;
            std::string key;
            std::string value;
        };

        using section = std::string;
        using key = std::string;
        using value = node;
        using nodes = std::unordered_map<key, value>;
        using serctions = std::unordered_map<section, nodes>;

    public:
        /**
         * @brief 类型名称
        */
        static constexpr const char* kClassname = "IniConfig";
        /**
         * @brief 构造
         * @param filename ini文件名
        */
        explicit IniConfig(const char* filename);
        ~IniConfig() = default;

        /**
         * @brief 获取指定部分中，指定k对应的值
         * @tparam OUT 返回值类型
         * @param sec []部分
         * @param k 键
         * @return 值
        */
        template <class OUT>
        OUT get(section sec, key k) const
        {
            const auto& siter = content_.find(sec);
            if (siter == content_.end()) {
                std::string except = stringop::str_format("配置文件:%s 没有节点:%s",
                    inifilename_.c_str(), sec.c_str());
                THROW_EXCEPTION(except);
            } else {
                const auto& niter = siter->second.find(k);
                if (niter == siter->second.end()) {
                    std::string except = stringop::str_format("配置文件:%s 在节点:%s 中不存在:%s",
                        inifilename_.c_str(), sec.c_str(), k.c_str());
                    THROW_EXCEPTION(except);
                } else {
                    OUT t;
                    if (niter->second.value.empty()) {
                        return t;
                    } else {
                        std::stringstream istream;
                        istream << niter->second.value;
                        istream >> t;
                        if (istream.eof() && !istream.fail()) {
                            return t;
                        } else {
                            std::string except = stringop::str_format("配置文件:%s 节点:%s key:%s 值:%s 转换失败",
                                inifilename_.c_str(), sec.c_str(), k.c_str(), niter->second.value.c_str());
                            THROW_EXCEPTION(except);
                        }
                    }
                }
            }
        }

    private:
        /**
         * @brief []部分
        */
        serctions content_;
        /**
         * @brief ini文件名
        */
        std::string inifilename_;
    };
}
}

#endif
