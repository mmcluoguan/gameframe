#include "client/role.h"

namespace client {

Role::Role()
{
    pmb_ = {
        { protocc::LOADROLE_CLIENT_GATE_S,
            std::bind(&Role::loadrole_client_gate_s, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::LOADGOODS_CLIENT_GATE_S,
            std::bind(&Role::loadgoods_client_gate_s, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::GOODSUPDATA_CLIENT_GATE_G,
            std::bind(&Role::goodsupdata_client_gate_g, this, std::placeholders::_1, std::placeholders::_2) },
    };
}

Role::~Role()
{
}

int Role::send_proto(int msgid, const google::protobuf::Message* data, std::stack<FilterData::Envelope>* enves, const std::string* extend) const
{
    std::shared_ptr<GateConnector> sh = gate_.lock();
    if (sh) {
        return sh->send_proto(msgid, data, enves, extend);
    } else {
        LOG_WARN << "角色:" << id_ << " 连接已断开";
    }
    return 0;
}

int Role::send_errcode(protocc::errnum code, const std::string& desc,
    std::stack<FilterData::Envelope>* enves)
{
    std::shared_ptr<GateConnector> sh = gate_.lock();
    if (sh) {
        return sh->send_errcode(code, desc, enves);
    } else {
        LOG_WARN << "角色:" << id_ << " 连接已断开";
    }
    return 0;
}

int Role::input_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<frmpub::FilterData::Envelope>> enves)
{
    auto it = pmb_.find(obj->msgid());
    if (it != pmb_.end()) {
        return it->second(obj, enves);
    } else {
        LOG_DEBUG << "消息" << frmpub::Basic::msgname(obj->msgid()) << " 没有处理函数";
    }
    return 0;
}

int Role::loadrole_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::loadrole_client_gate_s msgs;
    if (msgs.ParseFromString(data->msgdata()) == true) {
        LOG_DEBUG << "加载角色结果:" << msgs.result() << " roleid:" << msgs.roleid() << " level:" << msgs.level();
        id_ = msgs.roleid();
        level_ = msgs.level();

        protocc::loadgoods_client_gate_c msgc;
        msgc.set_roleid(id_);
        send_proto(protocc::LOADGOODS_CLIENT_GATE_C, &msgc);
        LOG_DEBUG << "加载角色物品数据";
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}
int Role::loadgoods_client_gate_s(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::loadgoods_client_gate_s msgs;
    if (msgs.ParseFromString(data->msgdata()) == true) {
        goodsmap_.clear();
        for (int i = 0; i < msgs.goods_size(); i++) {
            goodsmap_[msgs.goods(i).id()] = {
                msgs.goods(i).id(),
                msgs.goods(i).cfgid(),
                msgs.goods(i).num()
            };
            LOG_DEBUG << " 物品id:" << msgs.goods(i).id() << " cfgid:" << msgs.goods(i).cfgid() << " num:" << msgs.goods(i).num();
        }
        send_proto(protocc::NOTICE_INFO_LIST_CLENT_GATE_C);
        LOG_DEBUG << "获取广播公告信息列表";
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }

    return 0;
}

int Role::goodsupdata_client_gate_g(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::goodsupdata_client_gate_g msgs;
    if (msgs.ParseFromString(data->msgdata()) == true) {
        LOG_DEBUG << "角色物品变化 物品id:" << msgs.id() << " cfgid:" << msgs.cfgid() << " num:" << msgs.num();
        if (msgs.num() == 0) {
            goodsmap_.erase(msgs.id());
        } else {
            goods& gd = goodsmap_[msgs.id()];
            gd.cfgid = msgs.cfgid();
            gd.num = msgs.num();
        }
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}
}
