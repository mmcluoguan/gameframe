#ifndef FRMPUB_JSONCFG_H
#define FRMPUB_JSONCFG_H

#include "frmpub/matecfg.h"
#include "shynet/basic.h"
#include "shynet/utils/stringop.h"
#include <optional>

namespace frmpub {

/**
 * @brief json配置接口
*/
class JsoncfgBase {
public:
    /**
     * @brief 加载并反射json实体
     * @param root json文件所在目录
    */
    virtual void load(const char* root) = 0;
    /**
     * @brief 卸载json实体
    */
    virtual void unload() = 0;
    /**
     * @brief 重新加载json实体
     * @param root json文件所在目录
    */
    virtual void reload(const char* root) = 0;
    /**
     * @brief 获取json文件映射c++实体名称
     * @return json文件映射c++实体名称
    */
    virtual const char* name() = 0;
};

/**
 * @brief json配置
 * @tparam F json反射的c++实体类型
*/
template <typename F>
class JsonCfg final : public JsoncfgBase {
public:
    /**
     * @brief 加载并反射json实体
     * @param root json文件所在目录
    */
    void load(const char* root) override
    {
        const char* cfg = iguana_reflect_members(meta).name().data();
        std::string cfgstr = shynet::utils::stringop::str_format("%s/%s.json", root, cfg);
        std::ifstream jf(cfgstr);
        if (jf.is_open() == false) {
            std::ostringstream err;
            err << "打开json文件失败:" << cfgstr;
            THROW_EXCEPTION(err.str());
        }
        std::stringstream buffer;
        buffer << jf.rdbuf();
        bool ret = iguana::json::from_json(meta, buffer.str().c_str());
        if (ret == false) {
            std::ostringstream err;
            err << "josn反序列化失败:" << cfgstr << std::endl
                << iguana::json::error_desc();
            THROW_EXCEPTION(err.str());
        }
    }

    /**
     * @brief 卸载json实体
    */
    void unload() override
    {
        meta.data.clear();
    }

    /**
     * @brief 重新加载json实体
     * @param root json文件所在目录
    */
    void reload(const char* root) override
    {
        unload();
        load(root);
    }

    /**
     * @brief 通过key获取json记录反射的c++实体
     * @tparam R json记录反射的c++实体类型
     * @tparam K json记录反射的c++实体key的类型
     * @param id json记录反射的c++实体key值
     * @return json记录反射的c++实体
    */
    template <typename R, typename K>
    const std::optional<R> getById(K id)
    {
        std::optional<R> opt;
        auto it = meta.data.find(id);
        if (it == meta.data.end()) {
            return opt;
        }
        opt = it->second;
        return opt;
    }

    /**
     * @brief 获取json文件映射c++实体名称
     * @return json文件映射c++实体名称
    */
    const char* name() override
    {
        return iguana_reflect_members(meta).name().data();
    }

    /**
     * @brief 迭代json配置所有记录 
     * @tparam Fn 迭代函数类型
     * @param cb 迭代函数
    */
    template <typename Fn>
    void for_each(Fn cb)
    {
        for (const auto& it : meta.data) {
            cb(it.first, it.second);
        }
    }

private:
    /**
     * @brief json反射c++实体元类型
    */
    F meta {};
};
}

#endif
