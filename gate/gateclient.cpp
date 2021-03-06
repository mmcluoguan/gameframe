#include "gate/gateclient.h"
#include "3rd/fmt/format.h"
#include "frmpub/luacallbacktask.h"
#include "gate/connectormgr.h"
#include "gate/gateclientmgr.h"
#include "shynet/lua/luaengine.h"
#include "shynet/net/connectreactormgr.h"
#include "shynet/utils/elapsed.h"
#include "shynet/utils/iniconfig.h"
#include "shynet/utils/stringop.h"

//配置参数
extern const char* g_conf_node;

namespace gate {

GateClient::GateClient(std::shared_ptr<net::IPAddress> remote_addr,
    std::shared_ptr<net::IPAddress> listen_addr,
    std::shared_ptr<events::EventBuffer> iobuf)
    : frmpub::Client(remote_addr, listen_addr, iobuf,
        frmpub::NetConfigOptions {
            .type = SOCK_DGRAM,
            .enable_check = true,
            .pt = protocol::FilterProces::ProtoType::WEBSOCKET,
        })
{
    LOG_INFO << "新客户端连接 [ip:" << remote_addr->ip() << ":" << remote_addr->port() << "]";

    pmb_ = {
        { protocc::SERVERLIST_CLIENT_GATE_C,
            std::bind(&GateClient::serverlist_client_gate_c, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::SELECTSERVER_CLIENT_GATE_C,
            std::bind(&GateClient::selectserver_client_gate_c, this, std::placeholders::_1, std::placeholders::_2) },
    };
}

GateClient::~GateClient()
{
    LOG_INFO << (active() == net::CloseType::SERVER_CLOSE
                || active() == net::CloseType::TIMEOUT_CLOSE
            ? "服务器gate主动关闭连接 "
            : "客户端主动关闭连接 ")
             << "[ip:" << remote_addr()->ip() << ":" << remote_addr()->port() << "]";
}

void GateClient::preinput_handle(const std::shared_ptr<protocc::CommonObject> obj,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    LOG_INFO << "接收账号id:" << accountid_ << " 消息" << frmpub::Basic::msgname(obj->msgid());
}

int GateClient::default_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    if (obj->msgid() > protocc::CLIENT_LOGIN_BEGIN && obj->msgid() < protocc::CLIENT_LOGIN_END) {
        return login_message(obj, enves);
    } else if (obj->msgid() > protocc::CLIENT_GAME_BEGIN && obj->msgid() < protocc::CLIENT_GAME_END) {
        return game_message(obj, enves);
    } else if (obj->msgid() > protocc::CLIENT_GATE_BEGIN && obj->msgid() < protocc::CLIENT_GATE_END) {
        //通知lua的onMessage函数
        shynet::utils::Singleton<lua::LuaEngine>::instance().append(
            std::make_shared<frmpub::OnMessageTask<GateClient>>(
                std::dynamic_pointer_cast<GateClient>(FilterData::shared_from_this()), obj, enves));
    } else {
        std::stringstream stream;
        stream << "非法消息" << obj->msgid();
        SEND_ERR(protocc::ILLEGAL_UNKNOWN_MESSAGE, stream.str());
    }
    return 0;
}

void GateClient::close(net::CloseType active)
{
    frmpub::Client::close(active);
    if (accountid_.empty() == false) {
        ConnectorMgr& mgr = shynet::utils::Singleton<ConnectorMgr>::instance();
        mgr.reduce_count(login_id_);

        //通知服务器玩家下线
        LOG_DEBUG << "通知服务器玩家下线 账号id:" << accountid_;
        protocc::clioffline_gate_all_c msg;
        msg.set_aid(accountid_);
        msg.set_ip(remote_addr()->ip());
        msg.set_port(remote_addr()->port());
        auto world = mgr.world_connector();
        if (world != nullptr) {
            world->send_proto(protocc::CLIOFFLINE_GATE_ALL_C, &msg);
        }
        int login_connect_id = mgr.sid_conv_connect_id(login_id_);
        auto lg = mgr.login_connector(login_connect_id);
        if (lg != nullptr) {
            lg->send_proto(protocc::CLIOFFLINE_GATE_ALL_C, &msg);
        }
        int game_connect_id = mgr.sid_conv_connect_id(game_id_);
        auto gs = mgr.game_connector(game_connect_id);
        if (gs != nullptr) {
            gs->send_proto(protocc::CLIOFFLINE_GATE_ALL_C, &msg);
        }
    }
    shynet::utils::Singleton<GateClientMgr>::instance().remove(iobuf()->fd());
}

int GateClient::login_message(std::shared_ptr<protocc::CommonObject> obj,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    if (accountid_.empty() == true && obj->msgid() != protocc::LOGIN_CLIENT_GATE_C && obj->msgid() != protocc::RECONNECT_CLIENT_GATE_C) {
        SEND_ERR(protocc::UNAUTHENTICATED, "未验证的客户端连接,消息终止转发");
        return -1;
    }
    ConnectorMgr& connectMgr = shynet::utils::Singleton<ConnectorMgr>::instance();
    int login_connect_id = connectMgr.sid_conv_connect_id(login_id_);
    std::shared_ptr<LoginConnector> login = connectMgr.select_login(login_connect_id);
    if (login != nullptr) {
        if (login_connect_id != login->login_conncet_id()) {
            LOG_DEBUG << "负载均衡选择的login_connect_id:" << login->login_conncet_id();
            login_connect_id = login->login_conncet_id();
        }
        FilterData::Envelope enve;
        enve.fd = iobuf()->fd();
        enve.addr = *remote_addr()->sockaddr();
        enves->push(enve);
        if (obj->msgid() == protocc::LOGIN_CLIENT_GATE_C) {
            //同服顶号处理
            protocc::login_client_gate_c msgc;
            if (msgc.ParseFromString(obj->msgdata()) == true) {
                auto cli = shynet::utils::Singleton<GateClientMgr>::instance().find(msgc.platform_key());
                if (cli) {
                    protocc::repeatlogin_client_gate_s msgs;
                    msgs.set_aid(cli->accountid());
                    cli->send_proto(protocc::REPEATLOGIN_CLIENT_GATE_G, &msgs);
                    cli->close(net::CloseType::SERVER_CLOSE);
                }
                platform_key_ = msgc.platform_key();
            } else {
                std::stringstream stream;
                stream << "消息" << frmpub::Basic::msgname(obj->msgid()) << "解析错误";
                SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
            }

            //登陆消息中附加上选择的loginid,gameid,gateid
            auto logininfo = connectMgr.find_connect_data(login_connect_id);
            if (logininfo == nullptr) {
                std::stringstream stream;
                stream << "没有可用的" << frmpub::Basic::connectname(protocc::ServerType::LOGIN) << "连接";
                SEND_ERR(protocc::LOGIN_NOT_EXIST, stream.str());
                return 0;
            }
            int game_connect_id = connectMgr.sid_conv_connect_id(game_id_);
            auto gameinfo = connectMgr.find_connect_data(game_connect_id);

            shynet::utils::IniConfig& ini = shynet::utils::Singleton<shynet::utils::IniConfig>::get_instance();
            int gateid = ini.get<int>(g_conf_node, "sid");
            std::string extend = shynet::utils::stringop::str_format("%d,%d,%d",
                gateid, logininfo->sif.sid(), gameinfo ? gameinfo->sif.sid() : 0);
            obj->set_extend(extend);
        } else if (obj->msgid() == protocc::RECONNECT_CLIENT_GATE_C) {
            //断线重连信息
            protocc::reconnect_client_gate_c reconnect_msg;
            reconnect_msg.ParseFromString(obj->msgdata());
            accountid_ = reconnect_msg.aid();
            game_id_ = reconnect_msg.gameid();
            login_id_ = reconnect_msg.loginid();
            //断线重连消息中附加上选择的gateid
            shynet::utils::IniConfig& ini = shynet::utils::Singleton<shynet::utils::IniConfig>::get_instance();
            int gateid = ini.get<int>(g_conf_node, "sid");
            obj->set_extend(std::to_string(gateid));
        }
        login->send_proto(obj.get(), enves.get());
        LOG_DEBUG << "转发消息" << frmpub::Basic::msgname(obj->msgid())
                  << "到login[" << login->connect_addr()->ip() << ":"
                  << login->connect_addr()->port() << "]"
                  << " client fd:" << enve.fd;

    } else {
        std::stringstream stream;
        stream << "没有可用的" << frmpub::Basic::connectname(protocc::ServerType::LOGIN) << "连接";
        SEND_ERR(protocc::LOGIN_NOT_EXIST, stream.str());
    }
    return 0;
}

int GateClient::game_message(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    if (accountid_.empty() == true) {
        SEND_ERR(protocc::UNAUTHENTICATED, "未验证的客户端连接,消息终止转发");
        return -1;
    }
    int game_connect_id = shynet::utils::Singleton<ConnectorMgr>::instance().sid_conv_connect_id(game_id_);
    std::shared_ptr<GameConnector> game = shynet::utils::Singleton<ConnectorMgr>::instance().game_connector(game_connect_id);
    if (game != nullptr) {
        FilterData::Envelope enve;
        enve.fd = iobuf()->fd();
        enve.addr = *remote_addr()->sockaddr();
        enves->push(enve);

        game->send_proto(obj.get(), enves.get());
        LOG_DEBUG << "转发消息" << frmpub::Basic::msgname(obj->msgid())
                  << "到game[" << game->connect_addr()->ip() << ":"
                  << game->connect_addr()->port() << "]";
    } else {
        std::stringstream stream;
        stream << "没有可用的" << frmpub::Basic::connectname(protocc::ServerType::GAME) << "连接";
        SEND_ERR(protocc::GAME_NOT_EXIST, stream.str());
    }
    return 0;
}

int GateClient::serverlist_client_gate_c(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::serverlist_client_gate_s msgs;
    shynet::utils::Singleton<ConnectorMgr>::instance()
        .foreach_connect_datas([&](int key, ConnectorMgr::ConnectData value) {
            protocc::ServerInfo* sif = msgs.add_sifs();
            *sif = value.sif;
        });
    send_proto(protocc::SERVERLIST_CLIENT_GATE_S, &msgs);
    return 0;
}

int GateClient::selectserver_client_gate_c(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::selectserver_client_gate_c msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {
        login_id_ = msgc.loginid();
        game_id_ = msgc.gameid();

        protocc::selectserver_client_gate_s msgs;
        msgs.set_result(0);
        send_proto(protocc::SELECTSERVER_CLIENT_GATE_S, &msgs);
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}
}
