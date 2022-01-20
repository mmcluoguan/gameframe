#include "frmpub/client.h"
#include "shynet/net/timerreactormgr.h"
#include "shynet/utils/logger.h"

namespace frmpub {
Client::Client(std::shared_ptr<net::IPAddress> remote_addr,
    std::shared_ptr<net::IPAddress> listen_addr,
    std::shared_ptr<events::EventBuffer> iobuf,
    bool enable_ping, ssize_t heartSecond,
    protocol::FilterProces::ProtoType pt,
    FilterData::ProtoData pd)
    : net::AcceptNewFd(remote_addr, listen_addr, iobuf, pt, false)
    , FilterData(pd)
{
    enable_ping_ = enable_ping;
    heartSecond_ = heartSecond;
    filter_ = this;
    if (enable_ping_ == true) {
        std::shared_ptr<PingTimer> pt = std::make_shared<PingTimer>(
            timeval { heartSecond_, 0L }, this);
        shynet::utils::Singleton<net::TimerReactorMgr>::instance().add(pt);
        ping_timer_ = pt;
    }
}

Client::~Client()
{
    std::shared_ptr<PingTimer> pt = ping_timer_.lock();
    if (pt != nullptr) {
        pt->clean_filter();
    }
}

void Client::close(net::CloseType active)
{
    active_ = active;
}

void Client::timerout(net::CloseType active)
{
    LOG_INFO << "检测到与客户端没有心跳超时 [ip:" << remote_addr()->ip() << ":" << remote_addr()->port() << "]";
    close(active);
}

net::InputResult Client::input()
{
    //有数据接收到，因此延迟发送心跳计时器时间
    std::shared_ptr<PingTimer> pt = ping_timer_.lock();
    if (pt != nullptr) {
        pt->set_val({ heartSecond_, 0L });
    }
    return net::AcceptNewFd::input();
}

int Client::message_handle(char* original_data, size_t datalen)
{
    return FilterData::message_handle(original_data, datalen);
}

}
