#include "client/gateconnector.h"
#include "3rd/fmt/format.h"
#include "client/gatereconnctortimer.h"
#include "frmpub/reconnecttimer.h"
#include "shynet/events/streambuff.h"
#include "shynet/net/connectreactormgr.h"
#include "shynet/pool/threadpool.h"
#include "shynet/utils/elapsed.h"

namespace client {
GateConnector::GateConnector(std::shared_ptr<net::IPAddress> connect_addr,
    std::shared_ptr<DisConnectData> disconnect)
    : frmpub::Connector(connect_addr, "GateConnector", false, false, 5L, shynet::protocol::FilterProces::ProtoType::WEBSOCKET)
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
        { protocc::GMORDER_CLIENT_GATE_S,
            std::bind(&GateConnector::gmorder_client_gate_s, this, std::placeholders::_1, std::placeholders::_2) },
    };
}
GateConnector::~GateConnector()
{
    if (active() == net::CloseType::CLIENT_CLOSE) {
        LOG_INFO << "连接gate客户端主动关闭连接 "
                 << "[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
    } else if (active() == net::CloseType::SERVER_CLOSE) {
        LOG_INFO << "服务器gate主动关闭连接 "
                 << "[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
    } else if (active() == net::CloseType::CONNECT_FAIL) {
        LOG_INFO << "连接服务器gate失败 "
                 << "[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
    }
    if (active() != net::CloseType::CLIENT_CLOSE && enable_reconnect_ == true) {
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
            accountid_ = disconnect_->accountid;
            login_id_ = disconnect_->login_id;
            game_id_ = disconnect_->game_id;
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
    auto cb = [&]() {
        auto it = pmb_.find(obj->msgid());
        if (it != pmb_.end()) {
            return it->second(obj, enves);
        } else {
            if (role_ != 0) {
                return role_->input_handle(obj, enves);
            } else {
                LOG_DEBUG << "消息" << frmpub::Basic::msgname(obj->msgid()) << " 没有处理函数";
            }
        }
        return 0;
    };

#ifdef USE_DEBUG
    std::string str = fmt::format("工作线程单任务执行 {}", frmpub::Basic::msgname(obj->msgid()));
    shynet::utils::elapsed(str.c_str());
    return cb();
#elif
    return cb();
#endif
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
        if (err.code() == protocc::errnum::GAME_ROLE_NOT_EXIST) {
            //加载角色数据
            if (role_ != nullptr) {
                protocc::loadrole_client_gate_c msg;
                msg.set_aid(accountid_);
                msg.set_roleid(role_->id());
                send_proto(protocc::LOADROLE_CLIENT_GATE_C, &msg);
                LOG_DEBUG << "重新加载角色数据 roleid:" << role_->id();
            }
        }
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
        msgc.set_platform_key(platform_key_);
        send_proto(protocc::LOGIN_CLIENT_GATE_C, &msgc);
        LOG_DEBUG << "登陆 platform_key:" << platform_key_;
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
    static int count = 0;
    protocc::login_client_gate_s msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {
        LOG_DEBUG << count << "登录结果:" << msgc.result() << " aid:" << msgc.aid();
        count++;
        //return 0;
        if (msgc.result() == 0) {
            accountid_ = msgc.aid();
            if (msgc.roleid() == 0) {
                //创建角色
                protocc::createrole_client_gate_c msg;
                msg.set_aid(accountid_);
                send_proto(protocc::CREATEROLE_CLIENT_GATE_C, &msg);
                LOG_DEBUG << "创建角色";
            } else {
                //加载角色数据
                if (role_ == nullptr) {
                    role_ = std::make_shared<Role>();
                }
                role_->set_gate(shared_from_this());
                role_->set_id(msgc.roleid());
                role_->set_accountid(accountid_);
                protocc::loadrole_client_gate_c msg;
                msg.set_aid(accountid_);
                msg.set_roleid(msgc.roleid());
                send_proto(protocc::LOADROLE_CLIENT_GATE_C, &msg);
                LOG_DEBUG << "加载角色数据 roleid:" << role_->id()
                          << " accountid:" << accountid_
                          << " platform_key:" << platform_key_;
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
        } else {
            //选择服务器
            protocc::selectserver_client_gate_c msg;
            msg.set_loginid(login_id_);
            msg.set_gameid(game_id_);
            send_proto(protocc::SELECTSERVER_CLIENT_GATE_C, &msg);
            LOG_DEBUG << "重新选择服务器";
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
            if (role_ == nullptr) {
                role_ = std::make_shared<Role>();
            }
            role_->set_gate(shared_from_this());
            role_->set_id(msgc.roleid());
            role_->set_accountid(accountid_);
            //加载角色数据
            protocc::loadrole_client_gate_c msg;
            msg.set_roleid(role_->id());
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

int GateConnector::gmorder_client_gate_s(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::gmorder_client_gate_s msgs;
    if (msgs.ParseFromString(data->msgdata()) == true) {
        LOG_DEBUG << "gm命令:" << msgs.order() << " 操作结果:" << msgs.desc();
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}
}
