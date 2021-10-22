#include "gate/gameconnector.h"
#include "frmpub/luacallbacktask.h"
#include "frmpub/reconnecttimer.h"
#include "gate/connectormgr.h"
#include "gate/gateclientmgr.h"
#include "shynet/lua/luaengine.h"
#include "shynet/net/connectreactormgr.h"
#include "shynet/utils/iniconfig.h"

namespace gate {
GameConnector::GameConnector(std::shared_ptr<net::IPAddress> connect_addr)
    : frmpub::Connector(connect_addr, "GameConnector")
{

    pmb_ = {
        { protocc::ERRCODE,
            std::bind(&GameConnector::errcode, this, std::placeholders::_1, std::placeholders::_2) },
    };
}
GameConnector::~GameConnector()
{
    if (active() == net::ConnectEvent::CloseType::CLIENT_CLOSE) {
        LOG_INFO << "连接game客户端主动关闭连接 "
                 << "[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
    } else if (active() == net::ConnectEvent::CloseType::SERVER_CLOSE) {
        LOG_INFO << "服务器game主动关闭连接 "
                 << "[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
    } else if (active() == net::ConnectEvent::CloseType::CONNECT_FAIL) {
        LOG_INFO << "连接服务器game失败 "
                 << "[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
    }
}
void GameConnector::complete()
{
    LOG_INFO << "连接服务器game成功 [ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";

    //通知lua的onConnect函数
    shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
        std::make_shared<frmpub::OnConnectorTask<GameConnector>>(shared_from_this()));

    //向游戏服注册服务器信息
    protocc::register_gate_game_c msgc;
    protocc::ServerInfo* sif = msgc.mutable_sif();
    sif->set_ip(shynet::utils::Singleton<GateClientMgr>::instance().listen_addr().ip());
    sif->set_port(shynet::utils::Singleton<GateClientMgr>::instance().listen_addr().port());
    sif->set_st(protocc::ServerType::GATE);
    shynet::utils::IniConfig& ini = shynet::utils::Singleton<shynet::utils::IniConfig>::get_instance();
    int sid = ini.get<int, int>("gate", "sid", 1);
    sif->set_sid(sid);
    std::string name = ini.get<const char*, std::string>("gate", "name", "");
    sif->set_name(name);
    send_proto(protocc::REGISTER_GATE_GAME_C, &msgc);
}

int GameConnector::input_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    if (obj != nullptr) {
        //直接处理的游戏服消息
        auto it = pmb_.find(obj->msgid());
        if (it != pmb_.end()) {
            return it->second(obj, enves);
        } else {
            if (enves->empty() == false) {
                return forward_game_client_c(obj, enves);
            } else {
                //通知lua的onMessage函数
                shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
                    std::make_shared<frmpub::OnMessageTask<GameConnector>>(shared_from_this(), obj, enves));
            }
        }
    }
    return 0;
}

void GameConnector::close(net::ConnectEvent::CloseType active)
{
    //通知lua的onClose函数
    shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
        std::make_shared<frmpub::OnCloseTask>(fd()));

    shynet::utils::Singleton<ConnectorMgr>::instance().remove(game_connect_id_);
    Connector::close(active);

    //通知lua的onConnect函数
    shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
        std::make_shared<frmpub::OnConnectorTask<GameConnector>>(shared_from_this()));

    //向游戏服注册服务器信息
    protocc::register_gate_game_c msgc;
    protocc::ServerInfo* sif = msgc.mutable_sif();
    sif->set_ip(shynet::utils::Singleton<GateClientMgr>::instance().listen_addr().ip());
    sif->set_port(shynet::utils::Singleton<GateClientMgr>::instance().listen_addr().port());
    sif->set_st(protocc::ServerType::GATE);
    shynet::utils::IniConfig& ini = shynet::utils::Singleton<shynet::utils::IniConfig>::get_instance();
    int sid = ini.get<int, int>("gate", "sid", 1);
    sif->set_sid(sid);
    std::string name = ini.get<const char*, std::string>("gate", "name", "");
    sif->set_name(name);
    send_proto(protocc::REGISTER_GATE_GAME_C, &msgc);
}

int GameConnector::errcode(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
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

int GameConnector::forward_game_client_c(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    if (enves->empty() == false) {
        FilterData::Envelope& env = enves->top();
        enves->pop();
        std::shared_ptr<GateClient> client = shynet::utils::Singleton<GateClientMgr>::instance().find(env.fd);
        if (client != nullptr) {
            if (data->msgid() == protocc::CREATEROLE_CLIENT_GATE_S) {
                protocc::createrole_client_gate_s createrole;
                if (createrole.ParseFromString(data->msgdata()) == true) {
                    if (createrole.result() == 0) {
                        //通知login修改account关联role
                        ConnectorMgr& connectMgr = shynet::utils::Singleton<ConnectorMgr>::instance();
                        int login_connect_id = connectMgr.sid_conv_connect_id(client->set_login_id());
                        std::shared_ptr<LoginConnector> login = connectMgr.select_login(login_connect_id);
                        if (login != nullptr) {
                            login->send_proto(data.get(), enves.get());
                        } else {
                            std::stringstream stream;
                            stream << "没有可用的" << frmpub::Basic::connectname(protocc::ServerType::LOGIN) << "连接";
                            SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
                            return 0;
                        }
                    }
                } else {
                    std::stringstream stream;
                    stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
                    SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
                    return 0;
                }
            }
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
