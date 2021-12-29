#ifndef FRMPUB_JSONCFG_H
#define FRMPUB_JSONCFG_H

#include "frmpub/matecfg.h"
#include "shynet/basic.h"
#include "shynet/utils/stringop.h"
#include <optional>

namespace frmpub {

/**
 * @brief json���ýӿ�
*/
class JsoncfgBase {
public:
    /**
     * @brief ���ز�����jsonʵ��
     * @param root json�ļ�����Ŀ¼
    */
    virtual void load(const char* root) = 0;
    /**
     * @brief ж��jsonʵ��
    */
    virtual void unload() = 0;
    /**
     * @brief ���¼���jsonʵ��
     * @param root json�ļ�����Ŀ¼
    */
    virtual void reload(const char* root) = 0;
    /**
     * @brief ��ȡjson�ļ�ӳ��c++ʵ������
     * @return json�ļ�ӳ��c++ʵ������
    */
    virtual const char* name() = 0;
};

/**
 * @brief json����
 * @tparam F json�����c++ʵ������
*/
template <typename F>
class JsonCfg final : public JsoncfgBase {
public:
    /**
     * @brief ���ز�����jsonʵ��
     * @param root json�ļ�����Ŀ¼
    */
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

    /**
     * @brief ж��jsonʵ��
    */
    void unload() override
    {
        meta.data.clear();
    }

    /**
     * @brief ���¼���jsonʵ��
     * @param root json�ļ�����Ŀ¼
    */
    void reload(const char* root) override
    {
        unload();
        load(root);
    }

    /**
     * @brief ͨ��key��ȡjson��¼�����c++ʵ��
     * @tparam R json��¼�����c++ʵ������
     * @tparam K json��¼�����c++ʵ��key������
     * @param id json��¼�����c++ʵ��keyֵ
     * @return json��¼�����c++ʵ��
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
     * @brief ��ȡjson�ļ�ӳ��c++ʵ������
     * @return json�ļ�ӳ��c++ʵ������
    */
    const char* name() override
    {
        return iguana_reflect_members(meta).name().data();
    }

    /**
     * @brief ����json�������м�¼ 
     * @tparam Fn ������������
     * @param cb ��������
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
     * @brief json����c++ʵ��Ԫ����
    */
    F meta {};
};
}

#endif
