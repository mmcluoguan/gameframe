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
        { protocc::GOODSUPDATA_CLIENT_GATE_G,
            std::bind(&Role::goodsupdata_client_gate_g, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::NOTICE_INFO_CLENT_GATE_G,
            std::bind(&Role::notice_info_clent_gate_g, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::NOTICE_INFO_LIST_CLENT_GATE_S,
            std::bind(&Role::notice_info_list_clent_gate_s, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::EMAIL_NEW_CLIENT_GATE_G,
            std::bind(&Role::email_new_client_gate_g, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::SETLEVEL_CLIENT_GATE_S,
            std::bind(&Role::setlevel_client_gate_s, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::LOADEMAILS_CLIENT_GATE_S,
            std::bind(&Role::loademails_client_gate_s, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::LOOKEMAIL_CLIENT_GATE_S,
            std::bind(&Role::lookemail_client_gate_s, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::GETANNEX_CLIENT_GATE_S,
            std::bind(&Role::getannex_client_gate_s, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::BASEUPDATA_CLIENT_GATE_G,
            std::bind(&Role::baseupdata_client_gate_g, this, std::placeholders::_1, std::placeholders::_2) },
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
        LOG_DEBUG << "加载角色结果:" << msgs.result()
                  << " roleid:" << msgs.roleid()
                  << " level:" << msgs.level()
                  << " gold:" << msgs.gold()
                  << " diamond:" << msgs.diamond();
        id_ = msgs.roleid();
        level_ = msgs.level();
        gold_ = msgs.gold();
        diamond_ = msgs.diamond();

        send_proto(protocc::LOADGOODS_CLIENT_GATE_C);
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
            Goods& gd = goodsmap_[msgs.id()];
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

int Role::notice_info_clent_gate_g(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::notice_info_clent_gate_g gmsg;
    if (gmsg.ParseFromString(data->msgdata()) == true) {
        LOG_DEBUG << "区服广播信息 info:" << gmsg.info();
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}

int Role::notice_info_list_clent_gate_s(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::notice_info_list_clent_gate_s gmsg;
    if (gmsg.ParseFromString(data->msgdata()) == true) {
        LOG_DEBUG << "区服广播信息 len:" << gmsg.datas_size();
        for (auto& item : gmsg.datas()) {
            auto second = std::chrono::seconds(item.time());
            auto tp = std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>(second);
            auto tt = std::chrono::system_clock::to_time_t(tp);
            std::tm* tm_time = std::gmtime(&tt);
            char timebuf[30] = { 0 };
            strftime(timebuf, sizeof(timebuf), "%F_%T", tm_time);

            LOG_INFO_BASE << "  区服广播信息 info:" << item.info() << " 时间:" << timebuf;
        }
        LOG_DEBUG << "加载邮件列表";
        send_proto(protocc::LOADEMAILS_CLIENT_GATE_C);
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}

int Role::email_new_client_gate_g(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::email_new_client_gate_g gmsg;
    if (gmsg.ParseFromString(data->msgdata()) == true) {
        Email email;
        email.id = gmsg.id();
        emailsmap_[email.id] = email;
        LOG_DEBUG << "获取新邮件 id:" << gmsg.id();
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}

int Role::setlevel_client_gate_s(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::setlevel_client_gate_s gmsg;
    if (gmsg.ParseFromString(data->msgdata()) == true) {
        level_ = gmsg.level();
        LOG_DEBUG << "设置角色等级结果:" << gmsg.result();
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}
int Role::loademails_client_gate_s(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::loademails_client_gate_s gmsg;
    if (gmsg.ParseFromString(data->msgdata()) == true) {
        LOG_DEBUG << "邮件列表 len:" << gmsg.emails_size();
        for (auto& data : gmsg.emails()) {
            Email email;
            email.id = data.id();
            email.is_read = data.is_read();
            email.is_receive = data.is_receive();
            emailsmap_[email.id] = email;
            LOG_INFO_BASE << " id:" << email.id
                          << " is_read:" << email.is_read
                          << " is_receive:" << email.is_receive;
        }
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}

int Role::lookemail_client_gate_s(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::lookemail_client_gate_s gmsg;
    if (gmsg.ParseFromString(data->msgdata()) == true) {
        LOG_DEBUG << "查看邮件信息结果:" << gmsg.result();
        if (gmsg.result() == 0) {
            auto iter = emailsmap_.find(gmsg.id());
            if (iter != emailsmap_.end()) {
                Email& email = iter->second;
                email.id = gmsg.id();
                email.title = gmsg.title();
                email.info = gmsg.info();
                email.type = gmsg.type();
                email.time = gmsg.time();
                LOG_INFO_BASE << "id:" << email.id
                              << " title:" << email.title
                              << " info:" << email.info
                              << " type:" << email.type
                              << " time:" << email.time;
                if (gmsg.has_annex()) {
                    email.annex = std::make_shared<Email::Annex>();
                    email.annex->gold = gmsg.annex().gold();
                    email.annex->diamond = gmsg.annex().diamond();
                    email.annex->goodslist.clear();
                    LOG_INFO_BASE << " 附件 gold:" << email.annex->gold
                                  << " diamond:" << email.annex->diamond;
                    for (auto& item : gmsg.annex().goods()) {
                        Goods goods;
                        goods.cfgid = item.cfgid();
                        goods.num = item.num();
                        email.annex->goodslist.push_back(goods);
                        LOG_INFO_BASE << " 物品 cfgid:" << goods.cfgid
                                      << " num:" << goods.num;
                    }
                }
            }
        }
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}

int Role::getannex_client_gate_s(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::getannex_client_gate_s gmsg;
    if (gmsg.ParseFromString(data->msgdata()) == true) {
        LOG_DEBUG << "领取邮件附件结果 result:" << gmsg.result();
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}

int Role::baseupdata_client_gate_g(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::baseupdata_client_gate_g gmsg;
    if (gmsg.ParseFromString(data->msgdata()) == true) {
        LOG_DEBUG << "角色基础数据变化 level:" << gmsg.level()
                  << " gold:" << gmsg.gold()
                  << " diamond:" << gmsg.diamond();
        level_ = gmsg.level();
        gold_ = gmsg.gold();
        diamond_ = gmsg.diamond();
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}
}
