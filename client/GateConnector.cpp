#include "client/gateconnector.h"
#include "client/gatereconnctortimer.h"
#include "frmpub/reconnecttimer.h"
#include "shynet/events/streambuff.h"
#include "shynet/net/connectreactormgr.h"

namespace client {
GateConnector::GateConnector(std::shared_ptr<net::IPAddress> connect_addr,
    std::shared_ptr<DisConnectData> disconnect)
    : frmpub::Connector(connect_addr, "GateConnector", true, false, 5L, shynet::protocol::FilterProces::ProtoType::WEBSOCKET)
{
    disconnect_ = disconnect;
    if (disconnect_ != nullptr) {
        login_id_ = disconnect_->login_id;
        game_id_ = disconnect_->game_id;
        accountid_ = disconnect_->accountid;
    }

    pmb_ = {
        { protocc::ERRCODE,
            std::bind(&GateConnector::errcode, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::SERVERLIST_CLIENT_GATE_S,
            std::bind(&GateConnector::serverlist_client_gate_s, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::SELECTSERVER_CLIENT_GATE_S,
            std::bind(&GateConnector::selectserver_client_gate_s, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::LOGIN_CLIENT_GATE_S,
            std::bind(&GateConnector::login_client_gate_s, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::REPEATLOGIN_CLIENT_GATE_S,
            std::bind(&GateConnector::repeatlogin_client_gate_s, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::RECONNECT_CLIENT_GATE_S,
            std::bind(&GateConnector::reconnect_client_gate_s, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::CREATEROLE_CLIENT_GATE_S,
            std::bind(&GateConnector::createrole_client_gate_s, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::LOADROLE_CLIENT_GATE_S,
            std::bind(&GateConnector::loadrole_client_gate_s, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::LOADGOODS_CLIENT_GATE_S,
            std::bind(&GateConnector::loadgoods_client_gate_s, this, std::placeholders::_1, std::placeholders::_2) },
    };
}
GateConnector::~GateConnector()
{
    if (active() == net::ConnectEvent::CloseType::CLIENT_CLOSE) {
        LOG_INFO << "连接gate客户端主动关闭连接 "
                 << "[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
    } else if (active() == net::ConnectEvent::CloseType::SERVER_CLOSE) {
        LOG_INFO << "服务器gate主动关闭连接 "
                 << "[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
    } else if (active() == net::ConnectEvent::CloseType::CONNECT_FAIL) {
        LOG_INFO << "连接服务器gate失败 "
                 << "[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
    }
    if (active() != net::ConnectEvent::CloseType::CLIENT_CLOSE && enable_reconnect_ == true) {
        LOG_INFO << "3秒后开始重连";

        std::shared_ptr<GateReConnctorTimer> reconnect(
            new GateReConnctorTimer(connect_addr(), disconnect_data(), { 3L, 0L }));
        shynet::utils::Singleton<net::TimerReactorMgr>::instance().add(reconnect);
    }
}
void GateConnector::complete()
{
    LOG_INFO << "连接服务器gate成功 [ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
    if (disconnect_ == nullptr) {
        //获取服务器列表
        send_proto(protocc::SERVERLIST_CLIENT_GATE_C);
        LOG_DEBUG << "获取服务器列表";
    } else {
        if (disconnect_->accountid.empty()) {
            //获取服务器列表
            send_proto(protocc::SERVERLIST_CLIENT_GATE_C);
            LOG_DEBUG << "获取服务器列表";
        } else {
            //模拟断线重连
            protocc::reconnect_client_gate_c msg;
            msg.set_aid(disconnect_->accountid);
            msg.set_loginid(disconnect_->login_id);
            msg.set_gameid(disconnect_->game_id);
            send_proto(protocc::RECONNECT_CLIENT_GATE_C, &msg);
            LOG_DEBUG << "模拟断线重连";
        }
    }
}

int GateConnector::input_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    if (obj != nullptr) {
        auto it = pmb_.find(obj->msgid());
        if (it != pmb_.end()) {
            return it->second(obj, enves);
        } else {
            LOG_DEBUG << "消息" << frmpub::Basic::msgname(obj->msgid()) << " 没有处理函数";
        }
    }
    return 0;
}
std::shared_ptr<GateConnector::DisConnectData> GateConnector::disconnect_data()
{
    std::shared_ptr<GateConnector::DisConnectData> data = std::make_shared<GateConnector::DisConnectData>();
    data->login_id = login_id_;
    data->game_id = game_id_;
    data->accountid = accountid_;
    return data;
}
int GateConnector::errcode(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
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
int GateConnector::serverlist_client_gate_s(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::serverlist_client_gate_s msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {
        for (int i = 0; i < msgc.sifs_size(); i++) {
            const protocc::ServerInfo& sif = msgc.sifs(i);
            if (sif.st() == protocc::ServerType::LOGIN && login_id_ == 0) {
                login_id_ = sif.sid();
            } else if (sif.st() == protocc::ServerType::GAME && game_id_ == 0) {
                game_id_ = sif.sid();
            }
        }

        //选择服务器
        protocc::selectserver_client_gate_c msg;
        msg.set_loginid(login_id_);
        msg.set_gameid(game_id_);
        send_proto(protocc::SELECTSERVER_CLIENT_GATE_C, &msg);
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}
int GateConnector::selectserver_client_gate_s(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::selectserver_client_gate_s msgs;
    if (msgs.ParseFromString(data->msgdata()) == true) {
        protocc::login_client_gate_c msgc;
        msgc.set_platform_key("aaaa_123456");
        send_proto(protocc::LOGIN_CLIENT_GATE_C, &msgc);
        LOG_DEBUG << "登陆";
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}
int GateConnector::login_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::login_client_gate_s msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {
        LOG_DEBUG << "登录结果:" << msgc.result() << " aid:" << msgc.aid();
        if (msgc.result() == 0) {
            accountid_ = msgc.aid();
            roleid_ = msgc.roleid();
            if (roleid_ == 0) {
                //创建角色
                protocc::createrole_client_gate_c msg;
                msg.set_aid(accountid_);
                send_proto(protocc::CREATEROLE_CLIENT_GATE_C, &msg);
                LOG_DEBUG << "创建角色";
            } else {
                //加载角色数据
                protocc::loadrole_client_gate_c msg;
                msg.set_aid(accountid_);
                msg.set_roleid(roleid_);
                send_proto(protocc::LOADROLE_CLIENT_GATE_C, &msg);
                LOG_DEBUG << "加载角色数据 roleid:" << roleid_;
            }
        }
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}
int GateConnector::repeatlogin_client_gate_s(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    LOG_DEBUG << "此账号在其他设备上登录";
    enable_reconnect_ = false;
    return 0;
}
int GateConnector::reconnect_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::reconnect_client_gate_s msgs;
    if (msgs.ParseFromString(data->msgdata()) == true) {
        LOG_DEBUG << "断线重连结果:" << msgs.result();
        if (msgs.result() == 0) {
            protocc::setlevel_client_gate_c msgc;
            msgc.set_roleid(roleid_);
            send_proto(protocc::SETLEVEL_CLIENT_GATE_C, &msgc);
            LOG_DEBUG << "设置角色等级为1";
        } else {
            protocc::login_client_gate_c msgc;
            msgc.set_platform_key("aaaa_123456");
            send_proto(protocc::LOGIN_CLIENT_GATE_C, &msgc);
            LOG_DEBUG << "登陆";
        }
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}

int client::GateConnector::createrole_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::createrole_client_gate_s msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {
        LOG_DEBUG << "创建角色结果:" << msgc.result();
        if (msgc.result() == 0) {
            roleid_ = msgc.roleid();
            //加载角色数据
            protocc::loadrole_client_gate_c msg;
            msg.set_roleid(roleid_);
            send_proto(protocc::LOADROLE_CLIENT_GATE_C, &msg);
            LOG_DEBUG << "加载角色数据";
        }
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }

    return 0;
}

int client::GateConnector::loadrole_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::loadrole_client_gate_s msgs;
    if (msgs.ParseFromString(data->msgdata()) == true) {
        LOG_DEBUG << "加载角色结果:" << msgs.result() << " roleid:" << msgs.roleid() << " level:" << msgs.level();

        protocc::loadgoods_client_gate_c msgc;
        msgc.set_roleid(roleid_);
        send_proto(protocc::LOADGOODS_CLIENT_GATE_C, &msgc);
        LOG_DEBUG << "加载角色物品数据";
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}
int GateConnector::loadgoods_client_gate_s(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::loadgoods_client_gate_s msgs;
    if (msgs.ParseFromString(data->msgdata()) == true) {
        for (int i = 0; i < msgs.goods_size(); i++) {
            LOG_DEBUG << " 物品 id:" << msgs.goods(i).id() << " cfgid:" << msgs.goods(i).cfgid() << " num:" << msgs.goods(i).num();
        }
        protocc::setlevel_client_gate_c msgc;
        msgc.set_roleid(roleid_);
        send_proto(protocc::SETLEVEL_CLIENT_GATE_C, &msgc);
        LOG_DEBUG << "设置角色等级为1";
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }

    return 0;
}
}
