#include "admin_client/worldconnector.h"
#include "3rd/fmt/format.h"
#include "admin_client/connectormgr.h"
#include "frmpub/reconnecttimer.h"
#include "shynet/events/streambuff.h"
#include "shynet/net/connectreactormgr.h"
#include "shynet/utils/elapsed.h"
#include "shynet/utils/stuff.h"

namespace admin_client {
WorldConnector::WorldConnector(std::shared_ptr<net::IPAddress> connect_addr)
    : frmpub::Connector(connect_addr, "WorldConnector", true, false, 5L, shynet::protocol::FilterProces::ProtoType::HTTP, frmpub::FilterData::ProtoData::JSON)
{
    jmb_ = {
        { int(protocc::ERRCODE),
            std::bind(&WorldConnector::errcode, this, std::placeholders::_1, std::placeholders::_2) },
        { int(frmpub::JosnMsgId::GETGAMELIST_ADMIN_WORLD_S),
            std::bind(&WorldConnector::getgamelist_admin_world_s, this, std::placeholders::_1, std::placeholders::_2) },
        { int(frmpub::JosnMsgId::NOTICESERVER_ADMIN_WORLD_S),
            std::bind(&WorldConnector::noticeserver_admin_world_s, this, std::placeholders::_1, std::placeholders::_2) },
        { int(frmpub::JosnMsgId::SYSEMAIL_ADMIN_WORLD_S),
            std::bind(&WorldConnector::sysemail_admin_world_s, this, std::placeholders::_1, std::placeholders::_2) },
    };
}

WorldConnector::~WorldConnector()
{
    if (active() == net::CloseType::CLIENT_CLOSE) {
        LOG_INFO << "连接world客户端主动关闭连接 "
                 << "[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
    } else if (active() == net::CloseType::SERVER_CLOSE) {
        LOG_INFO << "服务器world主动关闭连接 "
                 << "[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
    } else if (active() == net::CloseType::CONNECT_FAIL) {
        LOG_INFO << "连接服务器world失败 "
                 << "[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
    }
    if (active() != net::CloseType::CLIENT_CLOSE) {
        LOG_INFO << "3秒后开始重连";
        std::shared_ptr<frmpub::ReConnectTimer<WorldConnector>> reconnect(
            new frmpub::ReConnectTimer<WorldConnector>(connect_addr(), { 3L, 0L }));
        shynet::utils::Singleton<net::TimerReactorMgr>::instance().add(reconnect);
    }
}
void WorldConnector::complete()
{
    LOG_INFO << "连接服务器world成功 [ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
    shynet::utils::Singleton<ConnectorMgr>::instance().add_worldctor(connectid());
}

int WorldConnector::input_handle(std::shared_ptr<rapidjson::Document> doc, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    int msgid = (*doc)["msgid"].GetInt();
    auto cb = [&]() {
        auto it = jmb_.find(msgid);
        if (it != jmb_.end()) {
            return it->second(doc, enves);
        } else {
            LOG_DEBUG << "消息" << frmpub::Basic::msgname(msgid) << " 没有处理函数";
        }
        return 0;
    };

#ifdef USE_DEBUG
    std::string str = fmt::format("工作线程单任务执行 {}", frmpub::Basic::msgname(msgid));
    shynet::utils::elapsed(str.c_str());
    return cb();
#elif
    return cb();
#endif
}

void WorldConnector::close(net::CloseType active)
{
    shynet::utils::Singleton<ConnectorMgr>::instance().remove_worldctor(connectid());
    Connector::close(active);
}

int WorldConnector::errcode(std::shared_ptr<rapidjson::Document> doc, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    try {
        rapidjson::Value& msgdata = frmpub::get_json_value(*doc, "msgdata");
        int32_t code = frmpub::get_json_value(msgdata, "code").GetInt();
        const char* desc = frmpub::get_json_value(msgdata, "desc").GetString();
        LOG_DEBUG << "错误码:" << code << " 描述:" << desc;
    } catch (const std::exception& err) {
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, err.what());
    }
    return 0;
}

int WorldConnector::getgamelist_admin_world_s(std::shared_ptr<rapidjson::Document> doc, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    try {
        rapidjson::Value& msgdata = frmpub::get_json_value(*doc, "msgdata");
        rapidjson::Value& games = frmpub::get_json_value(msgdata, "games");
        gamemap_.clear();
        LOG_DEBUG << "获取区服列表完成";
        for (auto& v : games.GetArray()) {
            auto obj = v.GetObject();
            protocc::ServerInfo sif;
            sif.set_ip(frmpub::get_json_value(v.GetObject(), "ip").GetString());
            sif.set_port(frmpub::get_json_value(v.GetObject(), "port").GetInt());
            int32_t st = frmpub::get_json_value(v.GetObject(), "st").GetInt();
            sif.set_st(static_cast<protocc::ServerType>(st));
            sif.set_sid(frmpub::get_json_value(v.GetObject(), "sid").GetInt());
            sif.set_name(frmpub::get_json_value(v.GetObject(), "name").GetString());
            gamemap_[sif.sid()] = sif;
            LOG_INFO_BASE << " ip:" << sif.ip()
                          << " port:" << sif.port()
                          << " 类型:" << frmpub::Basic::connectname(sif.st())
                          << " sid:" << sif.sid()
                          << " 名称:" << sif.name();
        }
    } catch (const std::exception& err) {
        shynet::utils::stuff::print_exception(err);
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, err.what());
    }
    return 0;
}

int WorldConnector::noticeserver_admin_world_s(std::shared_ptr<rapidjson::Document> doc, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    try {
        rapidjson::Value& msgdata = frmpub::get_json_value(*doc, "msgdata");
        int32_t result = frmpub::get_json_value(msgdata, "result").GetInt();
        LOG_DEBUG << "广播通知消息结果 result:" << result;
    } catch (const std::exception& err) {
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, err.what());
    }
    return 0;
}

int WorldConnector::sysemail_admin_world_s(std::shared_ptr<rapidjson::Document> doc, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    try {
        rapidjson::Value& msgdata = frmpub::get_json_value(*doc, "msgdata");
        int32_t result = frmpub::get_json_value(msgdata, "result").GetInt();
        LOG_DEBUG << "发送系统邮件结果 result:" << result;
    } catch (const std::exception& err) {
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, err.what());
    }
    return 0;
}
}
