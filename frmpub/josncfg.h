#ifndef FRMPUB_JSONCFG_H
#define FRMPUB_JSONCFG_H

#include "frmpub/matecfg.h"
#include "shynet/basic.h"

namespace frmpub {

class JsoncfgBase {
public:
    virtual void load(const char* root) = 0;
    virtual void unload() = 0;
    virtual void reload(const char* root) = 0;
    virtual const char* name() = 0;
};

template <typename F>
class JsonCfg final : public JsoncfgBase {
public:
    void load(const char* root) override
    {
        const char* cfg = iguana_reflect_members(meta).name().data();
        std::string cfgstr = shynet::utils::stringop::str_format("%s/%s.json", root, cfg);
        std::ifstream jf(cfgstr);
        if (jf.is_open() == false) {
            std::ostringstream err;
            err << "open failure:" << cfgstr;
            THROW_EXCEPTION(err.str());
        }
        std::stringstream buffer;
        buffer << jf.rdbuf();
        bool ret = iguana::json::from_json(meta, buffer.str().c_str());
        if (ret == false) {
            std::ostringstream err;
            err << "from_json failure:" << cfg;
            THROW_EXCEPTION(err.str());
        }
    }

    void unload() override
    {
        meta.data.clear();
    }

    void reload(const char* root) override
    {
        unload();
        load(root);
    }

    template <typename R, typename K>
    const R* getById(K id)
    {
        auto it = meta.data.find(id);
        if (it == meta.data.end()) {
            return nullptr;
        }
        return &(it->second);
    }

    const char* name() override
    {
        return iguana_reflect_members(meta).name().data();
    }

    template <typename Fn>
    void for_each(Fn cb)
    {
        for (const auto& it : meta.data) {
            cb(it.first, it.second);
        }
    }

    F meta {};
};
}

#endif
