#ifndef CLIENT_ROLE_H
#define CLIENT_ROLE_H

#include "client/gateconnector.h"
#include "frmpub/filterdata.h"
#include "shynet/basic.h"
#include "shynet/utils/singleton.h"
#include <list>

using namespace shynet;
using namespace frmpub;

namespace client {
class Role {

    friend class shynet::utils::Singleton<Role>;
    Role();

public:
    static constexpr const char* kClassname = "Role";

    struct Goods {
        int64_t id;
        int32_t cfgid;
        int32_t num;
    };

    struct Email {
        struct Annex {
            int gold;
            int diamond;
            std::list<Goods> goodslist;
        };

        int64_t id;
        bool is_read;
        bool is_receive;
        std::string title;
        std::string info;
        int type;
        int time;

        //附件
        std::shared_ptr<Annex> annex;
    };

    ~Role();

    int64_t id() const
    {
        return id_;
    }
    void set_id(int64_t v)
    {
        id_ = v;
    }

    std::string accountid() const
    {
        return accountid_;
    }
    void set_accountid(std::string v)
    {
        accountid_ = v;
    }

    int32_t level() const
    {
        return level_;
    }
    void set_level(int32_t v)
    {
        level_ = v;
    }

    int32_t gold() const
    {
        return gold_;
    }
    void set_gold(int32_t v)
    {
        gold_ = v;
    }

    int32_t diamond() const
    {
        return diamond_;
    }
    void set_diamond(int32_t v)
    {
        diamond_ = v;
    }

    int32_t lottery() const
    {
        return lottery_;
    }
    void set_lottery(int32_t v)
    {
        lottery_ = v;
    }

    void set_gate(std::weak_ptr<GateConnector> v)
    {
        gate_ = v;
    }

    std::unordered_map<int64_t, Goods>& goodsmap()
    {
        return goodsmap_;
    }

    int send_proto(int msgid, const google::protobuf::Message* data = nullptr,
        std::stack<FilterData::Envelope>* enves = nullptr, const std::string* extend = nullptr) const;

    int send_errcode(protocc::errnum code, const std::string& desc,
        std::stack<FilterData::Envelope>* enves = nullptr);

    int input_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<frmpub::FilterData::Envelope>> enves);

    /*
		* 加载角色数据结果
		*/
    int loadrole_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /*
		* 加载角色物品数据结果
		*/
    int loadgoods_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /*
	* 更新角色物品数据
	*/
    int goodsupdata_client_gate_g(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /*
		* 区服服务器广播信息
		*/
    int notice_info_clent_gate_g(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /*
		* 广播信息列表
		*/
    int notice_info_list_clent_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /*
		* 新邮件通知
		*/
    int email_new_client_gate_g(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /*
		* 设置角色等级
		*/
    int setlevel_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /*
		* 加载邮件列表
		*/
    int loademails_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /*
		* 查看邮件信息
		*/
    int lookemail_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /*
		* 领取邮件附件
		*/
    int getannex_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /*
		* 基础数据更新
		*/
    int baseupdata_client_gate_g(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

private:
    int64_t id_;
    std::string accountid_;
    std::weak_ptr<GateConnector> gate_;
    int32_t level_;
    int32_t gold_;
    int32_t diamond_;
    int32_t lottery_;
    std::unordered_map<int64_t, Goods> goodsmap_;
    std::unordered_map<int64_t, Email> emailsmap_;

    using ProtoMsgBind = std::function<int(std::shared_ptr<protocc::CommonObject> obj,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves)>;
    std::unordered_map<int, ProtoMsgBind> pmb_;
};
}

#endif
