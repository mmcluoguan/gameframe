#include "frmpub/logconnector.h"
#include "frmpub/logconnectormgr.h"
#include "frmpub/reconnecttimer.h"

namespace frmpub {
LogConnector::LogConnector(std::shared_ptr<net::IPAddress> connect_addr)
    : frmpub::Connector(connect_addr, "LogConnector")
{

    pmb_ = {
        { protocc::ERRCODE,
            std::bind(&LogConnector::errcode, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::WRITELOG_TO_LOG_S,
            std::bind(&LogConnector::writelog_to_log_s, this, std::placeholders::_1, std::placeholders::_2) },
    };
}
LogConnector::~LogConnector()
{
    if (active() == net::CloseType::CLIENT_CLOSE) {
        LOG_INFO << "连接log客户端主动关闭连接 "
                 << "[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
    } else if (active() == net::CloseType::SERVER_CLOSE) {
        LOG_INFO << "服务器log主动关闭连接 "
                 << "[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
    } else if (active() == net::CloseType::CONNECT_FAIL) {
        LOG_INFO << "连接服务器log失败 "
                 << "[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
    }
    if (active() != net::CloseType::CLIENT_CLOSE) {
        LOG_INFO << "3秒后开始重连";
        std::shared_ptr<frmpub::ReConnectTimer<LogConnector>> reconnect(
            new frmpub::ReConnectTimer<LogConnector>(connect_addr(), { 3L, 0L }));
        shynet::utils::Singleton<net::TimerReactorMgr>::instance().add(reconnect);
    }
}

void LogConnector::complete()
{
    LOG_INFO << "连接服务器log成功 [ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
    shynet::utils::Singleton<LogConnectorMgr>::instance().add_logctor(connectid());
    set_loggather();
}
int LogConnector::input_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    auto cb = [&]() {
        auto it = pmb_.find(obj->msgid());
        if (it != pmb_.end()) {
            return it->second(obj, enves);
        }
        return 0;
    };
#ifdef USE_DEBUG
    std::string str("工作线程单任务执行 ");
    str.append(frmpub::Basic::msgname(obj->msgid()));
    shynet::utils::elapsed(str.c_str());
    return cb();
#else
    return cb();
#endif
}

void LogConnector::close(net::CloseType active)
{
    shynet::utils::Singleton<LogConnectorMgr>::instance().remove_logctor(connectid());
    Connector::close(active);
}

int LogConnector::errcode(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
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

int LogConnector::writelog_to_log_s(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    /*protocc::writelog_to_log_s msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }*/
    return 0;
}
}
