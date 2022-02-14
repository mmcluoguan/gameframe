#include "gate/loginconnector.h"
#include "3rd/fmt/format.h"
#include "frmpub/luacallbacktask.h"
#include "frmpub/reconnecttimer.h"
#include "gate/connectormgr.h"
#include "gate/gateclientmgr.h"
#include "shynet/lua/luaengine.h"
#include "shynet/net/connectreactormgr.h"
#include "shynet/utils/elapsed.h"
#include "shynet/utils/iniconfig.h"

//配置参数
extern const char* g_conf_node;

namespace gate {

extern std::atomic<int> ncount;

LoginConnector::LoginConnector(std::shared_ptr<net::IPAddress> connect_addr)
    : frmpub::Connector(connect_addr, "LoginConnector")
{
    pmb_ = {
        { protocc::ERRCODE,
            std::bind(&LoginConnector::errcode, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::REPEATLOGIN_CLIENT_GATE_G,
            std::bind(&LoginConnector::repeatlogin_client_gate_g, this, std::placeholders::_1, std::placeholders::_2) },
    };
}
LoginConnector::~LoginConnector()
{
    if (active() == net::CloseType::CLIENT_CLOSE) {
        LOG_INFO << "连接login客户端主动关闭连接 "
                 << "[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
    } else if (active() == net::CloseType::SERVER_CLOSE) {
        LOG_INFO << "服务器login主动关闭连接 "
                 << "[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
    } else if (active() == net::CloseType::CONNECT_FAIL) {
        LOG_INFO << "连接服务器login失败 "
                 << "[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
    }
}
void LoginConnector::complete()
{
    LOG_INFO << "连接服务器login成功 [ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";

    //通知lua的onConnect函数
    shynet::utils::Singleton<lua::LuaEngine>::instance().append(
        std::make_shared<frmpub::OnConnectorTask<LoginConnector>>(
            std::dynamic_pointer_cast<LoginConnector>(shared_from_this())));

    //向登录服注册服务器信息
    protocc::register_gate_login_c msgc;
    protocc::ServerInfo* sif = msgc.mutable_sif();
    sif->set_ip(shynet::utils::Singleton<GateClientMgr>::instance().listen_addr().ip());
    sif->set_port(shynet::utils::Singleton<GateClientMgr>::instance().listen_addr().port());
    sif->set_st(protocc::ServerType::GATE);
    shynet::utils::IniConfig& ini = shynet::utils::Singleton<shynet::utils::IniConfig>::get_instance();
    int sid = ini.get<int>(g_conf_node, "sid");
    sif->set_sid(sid);
    std::string name = ini.get<std::string>(g_conf_node, "name");
    sif->set_name(name);
    send_proto(protocc::REGISTER_GATE_LOGIN_C, &msgc);
}
int LoginConnector::default_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    if (enves->empty() == false) {
        return forward_login_client_c(obj, enves);
    } else {
        //通知lua的onMessage函数
        shynet::utils::Singleton<lua::LuaEngine>::instance().append(
            std::make_shared<frmpub::OnMessageTask<LoginConnector>>(
                std::dynamic_pointer_cast<LoginConnector>(shared_from_this()), obj, enves));
    }
    return 0;
}
void LoginConnector::close(net::CloseType active)
{
    //通知lua的onClose函数
    shynet::utils::Singleton<lua::LuaEngine>::instance().append(
        std::make_shared<frmpub::OnCloseTask>(fd()));

    shynet::utils::Singleton<ConnectorMgr>::instance().remove(login_connect_id_);
    Connector::close(active);
}

int LoginConnector::errcode(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
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

int LoginConnector::forward_login_client_c(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    if (enves->empty() == false) {
        FilterData::Envelope& env = enves->top();
        enves->pop();
        std::shared_ptr<GateClient> client = shynet::utils::Singleton<GateClientMgr>::instance().find(env.fd);
        if (client != nullptr) {
            if (data->msgid() == protocc::LOGIN_CLIENT_GATE_S) {
                protocc::login_client_gate_s msgc;
                if (msgc.ParseFromString(data->msgdata()) == true) {
                    //assert(msgc.result() == 0);
                    if (msgc.result() == 0) {
                        //assert(msgc.aid().empty() == false);
                        client->set_accountid(msgc.aid());
                        if (data->extend().empty() == false) {
                            //从扩展信息中取出gamesid
                            client->set_game_id(atoi(data->extend().c_str()));
                        }
                    }
                } else {
                    std::stringstream stream;
                    stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
                    SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
                }
            } else if (data->msgid() == protocc::RECONNECT_CLIENT_GATE_S) {
                protocc::reconnect_client_gate_s msgc;
                if (msgc.ParseFromString(data->msgdata()) == true) {
                    if (msgc.result() == 0) {
                        client->set_accountid(msgc.aid());
                        client->set_login_id(msgc.loginid());
                        client->set_game_id(msgc.gameid());

                        //通知游戏服玩家断线重连成功
                        int conncetid = shynet::utils::Singleton<ConnectorMgr>::instance().sid_conv_connect_id(msgc.gameid());
                        auto game = shynet::utils::Singleton<ConnectorMgr>::instance().game_connector(conncetid);
                        if (game != nullptr) {
                            game->send_proto(data.get(), enves.get());
                            LOG_DEBUG << "通知游戏服玩家断线重连成功" << frmpub::Basic::msgname(data->msgid());
                        }
                    }
                } else {
                    std::stringstream stream;
                    stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
                    SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
                }
            }

            client->send_proto(data.get(), enves.get());
            LOG_DEBUG << "转发消息" << frmpub::Basic::msgname(data->msgid())
                      << "到client[" << client->remote_addr()->ip() << ":"
                      << client->remote_addr()->port() << "]"
                      << "账号id:" << client->accountid()
                      << " platform_key:" << client->platform_key();
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

int LoginConnector::repeatlogin_client_gate_g(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::repeatlogin_client_gate_s msg;
    if (msg.ParseFromString(data->msgdata()) == true) {
        std::shared_ptr<GateClient> client = shynet::utils::Singleton<GateClientMgr>::instance().find(msg.aid());
        if (client != nullptr) {
            //顶掉之前登录的账号
            client->send_proto(data.get(), enves.get());
            client->close(net::CloseType::SERVER_CLOSE);
        } else {
            std::stringstream stream;
            stream << "client acccount:" << msg.aid() << " 已断开连接";
            SEND_ERR(protocc::CLIENT_CLOSEED, stream.str());
        }
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}
}
