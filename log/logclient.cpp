#include "log/logclient.h"
#include "3rd/fmt/format.h"
#include "frmpub/protocc/log.pb.h"
#include "log/logclientmgr.h"
#include "log/logtimer.h"
#include "shynet/utils/elapsed.h"

extern std::weak_ptr<logs::LogTimer> g_logtimer;

namespace logs {
LogClient::LogClient(std::shared_ptr<net::IPAddress> remote_addr,
    std::shared_ptr<net::IPAddress> listen_addr,
    std::shared_ptr<events::EventBuffer> iobuf)
    : frmpub::Client(remote_addr, listen_addr, iobuf)
{
    LOG_INFO << "新客户端连接 [ip:" << remote_addr->ip() << ":" << remote_addr->port() << "]";

    pmb_ = {
        { protocc::ERRCODE,
            std::bind(&LogClient::errcode, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::WRITELOG_TO_LOG_C,
            std::bind(&LogClient::writelog_to_log_c, this, std::placeholders::_1, std::placeholders::_2) },
    };
}

LogClient::~LogClient()
{
    std::string str;
    if (active() == net::CloseType::SERVER_CLOSE) {
        str = "服务器log主动关闭连接";
    } else {
        str = "客户端主动关闭连接";
    }
    LOG_INFO << str << "[ip:" << remote_addr()->ip() << ":" << remote_addr()->port() << "]";
}

int LogClient::input_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    auto cb = [&]() {
        auto it = pmb_.find(obj->msgid());
        if (it != pmb_.end()) {
            return it->second(obj, enves);
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

void LogClient::close(net::CloseType active)
{
    frmpub::Client::close(active);
    shynet::utils::Singleton<LogClientMgr>::instance().remove(iobuf()->fd());
}

int LogClient::errcode(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
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

int LogClient::writelog_to_log_c(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::writelog_to_log_c logmsg;
    if (logmsg.ParseFromString(data->msgdata()) == true) {
        protocc::writelog_to_log_s msgs;
        auto timer = g_logtimer.lock();
        if (timer) {
            timer->add(logmsg);
            msgs.set_result(0);
        } else {
            msgs.set_result(1);
        }
        send_proto(protocc::WRITELOG_TO_LOG_S, &msgs);
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}

}
