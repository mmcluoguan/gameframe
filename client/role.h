#ifndef CLIENT_ROLE_H
#define CLIENT_ROLE_H

#include "client/gateconnector.h"
#include "frmpub/filterdata.h"
#include "shynet/basic.h"
#include "shynet/utils/singleton.h"

using namespace shynet;
using namespace frmpub;

namespace client {
class Role {

    friend class shynet::utils::Singleton<Role>;
    Role();

public:
    static constexpr const char* kClassname = "Role";

    struct goods {
        int64_t id;
        int32_t cfgid;
        int32_t num;
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

    void set_gate(std::weak_ptr<GateConnector> v)
    {
        gate_ = v;
    }

    std::unordered_map<int64_t, goods>& goodsmap()
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

private:
    int64_t id_;
    std::string accountid_;
    std::weak_ptr<GateConnector> gate_;
    int32_t level_;
    std::unordered_map<int64_t, goods> goodsmap_;

    using ProtoMsgBind = std::function<int(std::shared_ptr<protocc::CommonObject> obj,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves)>;
    std::unordered_map<int, ProtoMsgBind> pmb_;
};
}

#endif
