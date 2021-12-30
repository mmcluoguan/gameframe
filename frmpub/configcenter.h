#ifndef FRMPUB_CONFIGCENTER_H
#define FRMPUB_CONFIGCENTER_H

#include "frmpub/josncfg.h"
#include "shynet/utils/singleton.h"
#include <atomic>
#include <memory>
#include <string>

namespace frmpub {
/**
 * @brief 配置管理器
*/
class ConfigCenter final : public shynet::Nocopy {

    friend class shynet::utils::Singleton<ConfigCenter>;
    /**
     * @brief 构造
    */
    ConfigCenter() = default;

public:
    /**
     * @brief 加载数据保存的hash表数组索引
    */
    enum Index {
        Loaded = 0,
        ReLoaded = 1,
    };

    /**
    * @brief 类型名称
    */
    static constexpr const char* kClassname = "ConfigCenter";
    ~ConfigCenter() = default;

    /**
     * @brief 加载并反射json实体
     * @tparam T json反射c++实体元类型
     * @param index 加载数据保存hash表数组索引
    */
    template <typename T>
    void load(Index index = Loaded)
    {
        const char* name = iguana_reflect_members(T {}).name().data();
        if (cfg_hash_[index].find(name) == cfg_hash_[index].end()) {

            std::shared_ptr<JsonCfg<T>> cfg = std::make_shared<JsonCfg<T>>();
            cfg->load(search_path_.c_str());
            cfg_hash_[index][name] = cfg;
            use_index_ = index;
        }
    }

    /**
     * @brief 重新加载json实体
     * @tparam T json反射c++实体元类型
    */
    template <typename T>
    void reload()
    {
        const char* name = iguana_reflect_members(T {}).name().data();
        Index index = swap_index();
        auto it = cfg_hash_[index].find(name);
        if (it == cfg_hash_[index].end()) {
            load<T>(index);
        } else {
            it->second->reload(search_path_.c_str());
        }
        use_index_ = index;
    }

    /**
     * @brief 通过key获取json记录反射的c++实体
     * @tparam F json反射的c++实体类型
     * @tparam R json记录反射的c++实体类型
     * @tparam K json记录反射的c++实体key的类型
     * @param id json记录反射的c++实体key值
     * @return json记录反射的c++实体
    */
    template <typename F, typename R, typename K>
    const std::optional<R> get(K id)
    {
        std::optional<R> opt;
        const char* name = iguana_reflect_members(F {}).name().data();
        auto iter = cfg_hash_[use_index_].find(name);
        if (iter != cfg_hash_[use_index_].end()) {
            std::shared_ptr<JsonCfg<F>> meta = std::dynamic_pointer_cast<JsonCfg<F>>(iter->second);
            return meta->template getById<R>(id);
        }
        return opt;
    }

    /**
     * @brief 获取json文件所在目录
     * @return json文件所在目录
    */
    std::string search_path() { return search_path_; };
    /**
     * @brief 设置json文件所在目录
     * @param v json文件所在目录
    */
    void set_search_path(std::string v) { search_path_ = v; };

    /**
     * @brief 迭代json配置所有记录 
     * @tparam T json反射c++实体元类型
     * @tparam Fn 迭代函数类型
     * @param cb 迭代函数
    */
    template <typename T, typename Fn>
    void for_each(Fn cb)
    {
        const char* name = iguana_reflect_members(T {}).name().data();
        auto iter = cfg_hash_[use_index_].find(name);
        if (iter != cfg_hash_[use_index_].end()) {
            std::shared_ptr<JsonCfg<T>> meta = std::dynamic_pointer_cast<JsonCfg<T>>(iter->second);
            meta->for_each(cb);
        }
    }

private:
    /**
     * @brief 获取交替的索引
     * @return 交替的索引
    */
    Index swap_index() { return use_index_ == Loaded ? ReLoaded : Loaded; };
    /**
     * @brief json文件所在目录
    */
    std::string search_path_;
    /**
     * @brief 加载数据保存的hash表数组
    */
    std::unordered_map<std::string, std::shared_ptr<JsoncfgBase>> cfg_hash_[ReLoaded + 1];
    /**
     * @brief 当前使用加载数据保存的hash表数组的索引(用于reload时交换)
    */
    std::atomic<Index> use_index_ = Loaded;
};
}

#endif
