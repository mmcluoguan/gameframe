#include "login/loginclient.h"
#include "3rd/fmt/format.h"
#include "frmpub/luacallbacktask.h"
#include "login/connectormgr.h"
#include "login/loginclientmgr.h"
#include "shynet/lua/luaengine.h"
#include "shynet/utils/elapsed.h"

namespace login {
LoginClient::LoginClient(std::shared_ptr<net::IPAddress> remote_addr,
    std::shared_ptr<net::IPAddress> listen_addr,
    std::shared_ptr<events::EventBuffer> iobuf)
    : frmpub::Client(remote_addr, listen_addr, iobuf)
{
    LOG_INFO << "新客户端连接 [ip:" << remote_addr->ip() << ":" << remote_addr->port() << "]";

    pmb_ = {
        { protocc::ERRCODE,
            std::bind(&LoginClient::errcode, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::REGISTER_GATE_LOGIN_C,
            std::bind(&LoginClient::register_gate_login_c, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::LOGIN_CLIENT_GATE_C,
            std::bind(&LoginClient::login_client_gate_c, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::RECONNECT_CLIENT_GATE_C,
            std::bind(&LoginClient::forward_client_gate_c, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::CLIOFFLINE_GATE_ALL_C,
            std::bind(&LoginClient::clioffline_gate_all_c, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::CREATEROLE_CLIENT_GATE_S,
            std::bind(&LoginClient::createrole_client_gate_s, this, std::placeholders::_1, std::placeholders::_2) },
    };
}

LoginClient::~LoginClient()
{
    std::string str;
    if (active() == net::CloseType::SERVER_CLOSE) {
        str = "服务器login主动关闭连接";
    } else {
        str = frmpub::Basic::connectname(sif().st()) + "客户端主动关闭连接";
    }
    LOG_INFO << str << "[ip:" << remote_addr()->ip() << ":" << remote_addr()->port() << "]";
}

int LoginClient::default_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    //通知lua的onMessage函数
    shynet::utils::Singleton<lua::LuaEngine>::instance().append(
        std::make_shared<frmpub::OnMessageTask<LoginClient>>(
            std::dynamic_pointer_cast<LoginClient>(shared_from_this()), obj, enves));
    return 0;
}

void LoginClient::close(net::CloseType active)
{
    frmpub::Client::close(active);
    shynet::utils::Singleton<LoginClientMgr>::instance().remove(iobuf()->fd());
}

int LoginClient::errcode(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
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

int LoginClient::register_gate_login_c(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::register_gate_login_c msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {
        LOG_DEBUG << frmpub::Basic::connectname(protocc::ServerType::GATE) << "注册"
                  << " sid:" << msgc.sif().sid() << " ["
                  << msgc.sif().ip() << ":" << msgc.sif().port() << "]";
        set_sif(msgc.sif());
        protocc::register_gate_login_s msgs;
        msgs.set_result(0);
        send_proto(protocc::REGISTER_GATE_LOGIN_S, &msgs);
        LOG_DEBUG << "发送:REGISTER_GATE_LOGIN_S";
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}

int LoginClient::login_client_gate_c(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    //以后接第3方登录sdk
    return forward_client_gate_c(data, enves);
}

int LoginClient::forward_client_gate_c(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    std::shared_ptr<DbConnector> db = shynet::utils::Singleton<ConnectorMgr>::instance().db_connector();
    if (db != nullptr) {
        FilterData::Envelope enve;
        enve.fd = iobuf()->fd();
        enve.addr = *remote_addr()->sockaddr();
        enves->push(enve);
        db->send_proto(data.get(), enves.get());
        LOG_DEBUG << "转发消息" << frmpub::Basic::msgname(data->msgid())
                  << "到dbvisit[" << db->connect_addr()->ip() << ":"
                  << db->connect_addr()->port() << "]";
    } else {
        SEND_ERR_EX(protocc::DBVISIT_NOT_EXIST, "没有可选的db连接", enves.get());
    }
    return 0;
}
int LoginClient::clioffline_gate_all_c(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::clioffline_gate_all_c msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {
        return forward_client_gate_c(data, enves);
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}
int LoginClient::createrole_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::createrole_client_gate_s msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {
        std::shared_ptr<DbConnector> db = shynet::utils::Singleton<ConnectorMgr>::instance().db_connector();
        if (db != nullptr) {
            protocc::updata_to_dbvisit_c updata;
            updata.set_cache_key("account_" + msgc.aid());
            auto fields = updata.add_fields();
            fields->set_key("roleid");
            fields->set_value(std::to_string(msgc.roleid()));

            db->send_proto(protocc::UPDATA_TO_DBVISIT_C, &updata);
            LOG_DEBUG << "发送更新数据消息" << frmpub::Basic::msgname(data->msgid())
                      << "到dbvisit[" << db->connect_addr()->ip() << ":"
                      << db->connect_addr()->port() << "]";
        } else {
            SEND_ERR_EX(protocc::DBVISIT_NOT_EXIST, "没有可选的db连接", enves.get());
        }
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}
}
