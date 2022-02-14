#include "game/dbconnector.h"
#include "3rd/fmt/format.h"
#include "frmpub/luacallbacktask.h"
#include "frmpub/reconnecttimer.h"
#include "game/connectormgr.h"
#include "game/gameclientmgr.h"
#include "game/gameserver.h"
#include "shynet/lua/luaengine.h"
#include "shynet/net/connectreactormgr.h"
#include "shynet/utils/elapsed.h"
#include "shynet/utils/iniconfig.h"

//配置参数
extern const char* g_conf_node;

namespace game {
DbConnector::DbConnector(std::shared_ptr<net::IPAddress> connect_addr)
    : frmpub::Connector(connect_addr, "DbConnector")
{

    pmb_ = {
        { protocc::ERRCODE,
            std::bind(&DbConnector::errcode, this, std::placeholders::_1, std::placeholders::_2) },
    };
}
DbConnector::~DbConnector()
{
    if (active() == net::CloseType::CLIENT_CLOSE) {
        LOG_INFO << "连接dbvisit客户端主动关闭连接 "
                 << "[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
    } else if (active() == net::CloseType::SERVER_CLOSE) {
        LOG_INFO << "服务器dbvisit主动关闭连接 "
                 << "[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
    } else if (active() == net::CloseType::CONNECT_FAIL) {
        LOG_INFO << "连接服务器dbvisit失败 "
                 << "[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
    }
    if (active() != net::CloseType::CLIENT_CLOSE) {
        LOG_INFO << "3秒后开始重连";
        std::shared_ptr<frmpub::ReConnectTimer<DbConnector>> reconnect(
            new frmpub::ReConnectTimer<DbConnector>(connect_addr(), { 3L, 0L }));
        shynet::utils::Singleton<net::TimerReactorMgr>::instance().add(reconnect);
    }
}
void DbConnector::complete()
{
    LOG_INFO << "连接服务器dbvisit成功 [ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
    shynet::utils::Singleton<ConnectorMgr>::instance().add_dbctor(connectid());

    //通知lua的onConnect函数
    shynet::utils::Singleton<lua::LuaEngine>::instance().append(
        std::make_shared<frmpub::OnConnectorTask<DbConnector>>(
            std::dynamic_pointer_cast<DbConnector>(shared_from_this())));

    //向db服游戏服务器信息
    protocc::register_game_dbvisit_c msgc;
    protocc::ServerInfo* sif = msgc.mutable_sif();
    shynet::utils::IniConfig& ini = shynet::utils::Singleton<shynet::utils::IniConfig>::get_instance();
    std::string gameip = ini.get<std::string>(g_conf_node, "ip");
    short gameport = ini.get<short>(g_conf_node, "port");
    sif->set_ip(gameip);
    sif->set_port(gameport);
    sif->set_st(protocc::ServerType::GAME);
    int sid = ini.get<int>(g_conf_node, "sid");
    sif->set_sid(sid);
    std::string name = ini.get<std::string>(g_conf_node, "name");
    sif->set_name(name);
    send_proto({ protocc::REGISTER_GAME_DBVISIT_C, &msgc },
        { protocc::REGISTER_GAME_DBVISIT_S,
            [&](auto data, auto enves) -> int {
                return register_game_dbvisit_s(data, enves);
            } });
}
int DbConnector::default_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    //通知lua的onMessage函数
    shynet::utils::Singleton<lua::LuaEngine>::instance().append(
        std::make_shared<frmpub::OnMessageTask<DbConnector>>(
            std::dynamic_pointer_cast<DbConnector>(shared_from_this()), obj, enves));
    return 0;
}

void DbConnector::close(net::CloseType active)
{
    //通知lua的onClose函数
    shynet::utils::Singleton<lua::LuaEngine>::instance().append(
        std::make_shared<frmpub::OnCloseTask>(fd()));

    shynet::utils::Singleton<ConnectorMgr>::instance().remove_dbctor(connectid());
    Connector::close(active);
}

int DbConnector::errcode(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::errcode err;
    if (err.ParseFromString(data->msgdata()) == true) {
        LOG_DEBUG << "错误码:" << err.code() << " 描述:" << err.desc();
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}

int DbConnector::register_game_dbvisit_s(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::register_game_dbvisit_s msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {
        LOG_DEBUG << "游戏服注册结果:" << msgc.result();
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}
}
