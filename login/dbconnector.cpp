#include "login/dbconnector.h"
#include "3rd/fmt/format.h"
#include "frmpub/luacallbacktask.h"
#include "frmpub/reconnecttimer.h"
#include "login/connectormgr.h"
#include "login/loginclientmgr.h"
#include "login/loginserver.h"
#include "shynet/lua/luaengine.h"
#include "shynet/net/connectreactormgr.h"
#include "shynet/utils/elapsed.h"
#include "shynet/utils/iniconfig.h"

//配置参数
extern const char* g_conf_node;

namespace login {
DbConnector::DbConnector(std::shared_ptr<net::IPAddress> connect_addr)
    : frmpub::Connector(connect_addr, "DbConnector")
{
    pmb_ = {
        { protocc::ERRCODE,
            std::bind(&DbConnector::errcode, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::REGISTER_LOGIN_DBVISIT_S,
            std::bind(&DbConnector::register_login_dbvisit_s, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::REPEATLOGIN_CLIENT_GATE_S,
            std::bind(&DbConnector::repeatlogin_client_gate_s, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::LOGIN_CLIENT_GATE_S,
            std::bind(&DbConnector::login_client_gate_s, this, std::placeholders::_1, std::placeholders::_2) },
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
    shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
        std::make_shared<frmpub::OnConnectorTask<DbConnector>>(shared_from_this()));

    //向db服注册服务器信息
    protocc::register_login_dbvisit_c msgc;
    protocc::ServerInfo* sif = msgc.mutable_sif();
    shynet::utils::IniConfig& ini = shynet::utils::Singleton<shynet::utils::IniConfig>::get_instance();
    std::string loginip = ini.get<std::string>(g_conf_node, "ip");
    short loginport = ini.get<short>(g_conf_node, "port");
    sif->set_ip(loginip);
    sif->set_port(loginport);
    sif->set_st(protocc::ServerType::LOGIN);
    int sid = ini.get<int>(g_conf_node, "sid");
    sif->set_sid(sid);
    std::string name = ini.get<std::string>(g_conf_node, "name");
    sif->set_name(name);
    send_proto(protocc::REGISTER_LOGIN_DBVISIT_C, &msgc);
}

int DbConnector::input_handle(std::shared_ptr<protocc::CommonObject> obj,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    auto cb = [&]() {
        auto it = pmb_.find(obj->msgid());
        if (it != pmb_.end()) {
            return it->second(obj, enves);
        } else {
            if (enves->empty() == false) {
                return forward_db_gate_c(obj, enves);
            } else {
                //通知lua的onMessage函数
                shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
                    std::make_shared<frmpub::OnMessageTask<DbConnector>>(shared_from_this(), obj, enves));
            }
        }
        return 0;
    };
#ifdef USE_DEBUG
    std::string str = fmt::format("工作线程单任务执行 {}", frmpub::Basic::msgname(obj->msgid()));
    shynet::utils::elapsed(str.c_str());
    return cb();
#else
    return cb();
#endif
}

void DbConnector::close(net::CloseType active)
{
    //通知lua的onClose函数
    shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
        std::make_shared<frmpub::OnCloseTask>(fd()));

    shynet::utils::Singleton<ConnectorMgr>::instance().remove_dbctor(connectid());
    Connector::close(active);
}

int DbConnector::errcode(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
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

int DbConnector::forward_db_gate_c(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    if (enves->empty() == false) {
        FilterData::Envelope& env = enves->top();
        enves->pop();
        std::shared_ptr<LoginClient> gate = shynet::utils::Singleton<LoginClientMgr>::instance().find(env.fd);
        if (gate != nullptr) {
            gate->send_proto(data.get(), enves.get());
            LOG_DEBUG << "转发消息" << frmpub::Basic::msgname(data->msgid())
                      << "到gate[" << gate->remote_addr()->ip() << ":"
                      << gate->remote_addr()->port() << "]"
                      << " gate fd:" << env.fd;
        } else {
            std::stringstream stream;
            stream << "gate fd:" << env.fd << " 已断开连接";
            SEND_ERR(protocc::GATE_NOT_EXIST, stream.str());
        }
    } else {
        SEND_ERR(protocc::NO_ROUTING_INFO, "转发消息没有路由信息");
    }
    return 0;
}

int DbConnector::register_login_dbvisit_s(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::register_login_dbvisit_s msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {
        if (msgc.result() == 0) {
        }
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}

int DbConnector::repeatlogin_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    auto gate = shynet::utils::Singleton<LoginClientMgr>::instance().find_from_sid(data->extend());
    if (gate != nullptr) {
        gate->send_proto(data.get(), enves.get());
    } else {
        std::stringstream stream;
        stream << "没有可用的" << frmpub::Basic::connectname(protocc::ServerType::GATE) << "连接";
        SEND_ERR(protocc::GATE_NOT_EXIST, stream.str());
    }
    return 0;
}

int DbConnector::login_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::login_client_gate_s msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {
        //assert(msgc.aid().empty() == false);
        if (msgc.result() == 0) {
            //判断登录前是否选择gamesid
            if (data->extend().empty() == false) {
                if (data->extend() == "0") {
                    //请求world选择gamesid
                    auto world = shynet::utils::Singleton<ConnectorMgr>::instance().world_connector();
                    if (world != nullptr) {
                        world->send_proto(data.get(), enves.get());
                    } else {
                        std::stringstream stream;
                        stream << "没有可用的" << frmpub::Basic::connectname(protocc::ServerType::WORLD) << "连接";
                        SEND_ERR(protocc::WORLD_NOT_EXIST, stream.str());
                    }
                } else {
                    return forward_db_gate_c(data, enves);
                }
            } else {
                std::stringstream stream;
                stream << "附加信息解析错误 extend:" << data->extend();
                SEND_ERR(protocc::EXTEND_FORMAT_ERR, stream.str());
            }
        } else {
            return forward_db_gate_c(data, enves);
        }
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}
}
