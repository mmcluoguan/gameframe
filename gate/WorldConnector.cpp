#include "gate/WorldConnector.h"
#include "frmpub/LuaCallBackTask.h"
#include "frmpub/ReConnectTimer.h"
#include "gate/ConnectorMgr.h"
#include "gate/GameConnector.h"
#include "gate/GateClientMgr.h"
#include "gate/LoginConnector.h"
#include "shynet/lua/LuaEngine.h"
#include "shynet/net/ConnectReactorMgr.h"
#include "shynet/utils/IniConfig.h"

namespace gate {
WorldConnector::WorldConnector(std::shared_ptr<net::IPAddress> connect_addr)
    : frmpub::Connector(connect_addr, "WorldConnector")
{

    pmb_ = {
        { protocc::ERRCODE,
            std::bind(&WorldConnector::errcode, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::REGISTER_GATE_WORLD_S,
            std::bind(&WorldConnector::register_gate_world_s, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::SERONLINE_WORLD_GATE_G,
            std::bind(&WorldConnector::seronline_world_gate_g, this, std::placeholders::_1, std::placeholders::_2) },
    };
}
WorldConnector::~WorldConnector()
{
    if (active() == net::ConnectEvent::CloseType::CLIENT_CLOSE) {
        LOG_INFO << "连接world客户端主动关闭连接 "
                 << "[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
    } else if (active() == net::ConnectEvent::CloseType::SERVER_CLOSE) {
        LOG_INFO << "服务器world主动关闭连接 "
                 << "[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
    } else if (active() == net::ConnectEvent::CloseType::CONNECT_FAIL) {
        LOG_INFO << "连接服务器world失败 "
                 << "[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
    }
    if (active() != net::ConnectEvent::CloseType::CLIENT_CLOSE) {
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

    //通知lua的onConnect函数
    shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
        std::make_shared<frmpub::OnConnectorTask<WorldConnector>>(shared_from_this()));

    //向世界服注册网关服信息
    protocc::register_gate_world_c msgc;
    protocc::ServerInfo* sif = msgc.mutable_sif();
    sif->set_ip(shynet::utils::Singleton<GateClientMgr>::instance().listen_addr().ip());
    sif->set_port(shynet::utils::Singleton<GateClientMgr>::instance().listen_addr().port());
    sif->set_st(protocc::ServerType::GATE);
    shynet::utils::IniConfig& ini = shynet::utils::Singleton<shynet::utils::IniConfig>::get_instance();
    int sid = ini.get<int, int>("gate", "sid", 1);
    sif->set_sid(sid);
    std::string name = ini.get<const char*, std::string>("gate", "name", "");
    sif->set_name(name);
    send_proto(protocc::REGISTER_GATE_WORLD_C, &msgc);
}
int WorldConnector::input_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    if (obj != nullptr) {
        auto it = pmb_.find(obj->msgid());
        if (it != pmb_.end()) {
            return it->second(obj, enves);
        } else {
            if (enves->empty() == false) {
                return forward_world_client_c(obj, enves);
            } else {
                //通知lua的onMessage函数
                shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
                    std::make_shared<frmpub::OnMessageTask<WorldConnector>>(shared_from_this(), obj, enves));
            }
        }
    }
    return 0;
}

void WorldConnector::close(net::ConnectEvent::CloseType active)
{
    //通知lua的onClose函数
    shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
        std::make_shared<frmpub::OnCloseTask>(fd()));

    shynet::utils::Singleton<ConnectorMgr>::instance().remove_worldctor(connectid());
    Connector::close(active);
}

int WorldConnector::errcode(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::errcode err;
    if (err.ParseFromString(data->msgdata()) == true) {
        LOG_DEBUG << "错误码:" << err.code() << " 描述:" << err.desc();
        if (enves->empty() == false) {
            FilterData::Envelope& env = enves->top();
            enves->pop();
            std::shared_ptr<GateClient> client = shynet::utils::Singleton<GateClientMgr>::instance().find(env.fd);
            if (client != nullptr) {
                client->send_proto(data.get(), enves.get());
            } else {
                std::stringstream stream;
                stream << "client fd:" << env.fd << " 已断开连接";
                SEND_ERR(protocc::CLIENT_CLOSEED, stream.str());
            }
        }
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}

int WorldConnector::register_gate_world_s(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::register_gate_world_s msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {
        //遍历在线服务器列表
        for (int i = 0; i < msgc.sifs_size(); i++) {
            const protocc::ServerInfo& sif = msgc.sifs(i);
            if (sif.st() == protocc::ServerType::GAME) {
                if (shynet::utils::Singleton<net::ConnectReactorMgr>::instance()
                        .find(sif.ip(), (unsigned short)sif.port())
                    == nullptr) {
                    std::shared_ptr<GameConnector> game(
                        new GameConnector(std::shared_ptr<net::IPAddress>(
                            new net::IPAddress(sif.ip().c_str(), (unsigned short)sif.port()))));
                    //连接游戏服务器
                    int gameid = shynet::utils::Singleton<net::ConnectReactorMgr>::instance().add(game);
                    game->game_conncet_id(gameid);

                    shynet::utils::Singleton<ConnectorMgr>::instance().add_new_connect_data(gameid, sif);
                }
            } else if (sif.st() == protocc::ServerType::LOGIN) {
                if (shynet::utils::Singleton<net::ConnectReactorMgr>::instance()
                        .find(sif.ip(), (unsigned short)sif.port())
                    == nullptr) {
                    std::shared_ptr<LoginConnector> login(
                        new LoginConnector(std::shared_ptr<net::IPAddress>(
                            new net::IPAddress(sif.ip().c_str(), (unsigned short)sif.port()))));
                    //连接登录服务器
                    int loginid = shynet::utils::Singleton<net::ConnectReactorMgr>::instance().add(login);
                    login->login_conncet_id(loginid);

                    shynet::utils::Singleton<ConnectorMgr>::instance().add_new_connect_data(loginid, sif);
                }
            } else {
                std::stringstream stream;
                stream << "未知的服务器类型 st:" << sif.st();
                SEND_ERR(protocc::UNKNOWN_SERVER_TYPE, stream.str());
            }
        }
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}

int WorldConnector::seronline_world_gate_g(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::seronline_world_gate_g msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {
        const protocc::ServerInfo& sif = msgc.sif();
        if (sif.st() == protocc::ServerType::GAME) {
            if (shynet::utils::Singleton<net::ConnectReactorMgr>::instance()
                    .find(sif.ip(), (unsigned short)sif.port())
                == nullptr) {
                std::shared_ptr<GameConnector> game(
                    new GameConnector(std::shared_ptr<net::IPAddress>(
                        new net::IPAddress(sif.ip().c_str(), (unsigned short)sif.port()))));
                //连接游戏服务器
                int gameid = shynet::utils::Singleton<net::ConnectReactorMgr>::instance().add(game);
                game->game_conncet_id(gameid);

                shynet::utils::Singleton<ConnectorMgr>::instance().add_new_connect_data(gameid, sif);
            }
        } else if (sif.st() == protocc::ServerType::LOGIN) {
            if (shynet::utils::Singleton<net::ConnectReactorMgr>::instance()
                    .find(sif.ip(), (unsigned short)sif.port())
                == nullptr) {
                std::shared_ptr<LoginConnector> login(
                    new LoginConnector(std::shared_ptr<net::IPAddress>(
                        new net::IPAddress(sif.ip().c_str(), (unsigned short)sif.port()))));
                //连接登录服务器
                int loginid = shynet::utils::Singleton<net::ConnectReactorMgr>::instance().add(login);
                login->login_conncet_id(loginid);

                shynet::utils::Singleton<ConnectorMgr>::instance().add_new_connect_data(loginid, sif);
            }
        } else {
            std::stringstream stream;
            stream << "未知的服务器类型 st:" << sif.st();
            SEND_ERR(protocc::UNKNOWN_SERVER_TYPE, stream.str());
        }
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}

int WorldConnector::forward_world_client_c(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    if (enves->empty() == false) {
        FilterData::Envelope& env = enves->top();
        enves->pop();
        std::shared_ptr<GateClient> client = shynet::utils::Singleton<GateClientMgr>::instance().find(env.fd);
        if (client != nullptr) {
            client->send_proto(data.get(), enves.get());
            LOG_DEBUG << "转发消息" << frmpub::Basic::msgname(data->msgid())
                      << "到client[" << client->remote_addr()->ip() << ":"
                      << client->remote_addr()->port() << "]";
        } else {
            std::stringstream stream;
            stream << "client fd:" << env.fd << " 已断开连接";
            SEND_ERR(protocc::CLIENT_CLOSEED, stream.str());
        }
    } else {
        SEND_ERR(protocc::NO_ROUTING_INFO, "转发消息没有路由信息");
    }
    return 0;
}
}
