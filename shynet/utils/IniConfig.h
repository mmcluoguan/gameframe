#ifndef SHYNET_UTILS_INICONFIG_H
#define SHYNET_UTILS_INICONFIG_H

#include "shynet/basic.h"
#include <sstream>
#include <unordered_map>

namespace shynet {
namespace utils {
    class IniConfig final : public Nocopy {
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
        static constexpr const char* kClassname = "IniConfig";
        explicit IniConfig(const char* filename);
        ~IniConfig();

        template <class IN, class OUT>
        OUT get(section sec, key k, const IN& defv) const
        {
            const auto& siter = content_.find(sec);
            if (siter == content_.end()) {
                return defv;
            } else {
                const auto& niter = siter->second.find(k);
                if (niter == siter->second.end()) {
                    return defv;
                } else {
                    std::stringstream istream;
                    istream << niter->second.value;
                    OUT t;
                    istream >> t;
                    if (istream.eof() && !istream.fail()) {
                        return t;
                    } else {
                        return defv;
                    }
                }
            }
            return defv;
        }

    private:
        serctions content_;
    };
}
}

#endif
