#ifndef FRMPUB_CONFIGCENTER_H
#define FRMPUB_CONFIGCENTER_H

#include "frmpub/josncfg.h"
#include "shynet/utils/singleton.h"
#include <memory>
#include <string>

namespace frmpub {
class ConfigCenter final : public shynet::Nocopy {

    friend class shynet::utils::Singleton<ConfigCenter>;
    ConfigCenter() = default;

public:
    /**
    * @brief 类型名称
    */
    static constexpr const char* kClassname = "ConfigCenter";
    ~ConfigCenter() = default;

    template <typename T>
    void load(uint8_t index = 0)
    {
        const char* name = iguana_reflect_members(T {}).name().data();
        if (cfg_hash_[index].find(name) == cfg_hash_[index].end()) {

            std::shared_ptr<JsonCfg<T>> cfg = std::make_shared<JsonCfg<T>>();
            cfg->load(search_path_.c_str());
            cfg_hash_[index][name] = cfg;
            use_index = index;
        }
    }

    template <typename T>
    void reload()
    {
        const char* name = iguana_reflect_members(T {}).name().data();
        uint8_t index = reload_index();
        auto it = cfg_hash_[index].find(name);
        if (it == cfg_hash_[index].end()) {
            load<T>(index);
        } else {
            it->second->reload(search_path_.c_str());
        }
        use_index = index;
    }

    template <typename F, typename R, typename K>
    const R* get(K id)
    {
        const char* name = iguana_reflect_members(F {}).name().data();
        auto iter = cfg_hash_[use_index].find(name);
        if (iter != cfg_hash_[use_index].end()) {
            std::shared_ptr<JsonCfg<F>> meta = std::dynamic_pointer_cast<JsonCfg<F>>(iter->second);
            return meta->template getById<R>(id);
        }
        return nullptr;
    }

    std::string search_path() { return search_path_; };
    void set_search_path(std::string v) { search_path_ = v; };

    template <typename T, typename Fn>
    void for_each(Fn cb)
    {
        const char* name = iguana_reflect_members(T {}).name().data();
        auto iter = cfg_hash_[use_index].find(name);
        if (iter != cfg_hash_[use_index].end()) {
            std::shared_ptr<JsonCfg<T>> meta = std::dynamic_pointer_cast<JsonCfg<T>>(iter->second);
            meta->for_each(cb);
        }
    }

private:
    uint8_t reload_index() { return use_index == 0 ? 1 : 0; };

    std::string search_path_;
    std::unordered_map<std::string, std::shared_ptr<JsoncfgBase>> cfg_hash_[2];
    uint8_t use_index = 0;
};
}

#endif
