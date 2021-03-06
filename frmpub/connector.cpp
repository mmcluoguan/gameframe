#include "frmpub/connector.h"
#include "shynet/net/connectreactormgr.h"
#include "shynet/net/timerreactormgr.h"

namespace frmpub {
Connector::Connector(std::shared_ptr<net::IPAddress> connect_addr,
    std::string name,
    NetConfigOptions opt)
    : net::ConnectEvent(connect_addr, opt.pt, opt.type, opt.enable_ssl, opt.enable_check, opt.check_second)
    , FilterData(opt.pd)
{
    name_ = name;
    enable_ping_ = opt.enable_ping;
    heartSecond_ = opt.heartSecond;
    filter_ = this;
}
Connector::~Connector()
{
    std::shared_ptr<PingTimer> pt = ping_timer_.lock();
    if (pt != nullptr) {
        pt->clean_filter();
    }
}
void Connector::success()
{
    net::ConnectEvent::success();
    if (enable_ping_ == true) {
        std::shared_ptr<PingTimer> pt = std::make_shared<PingTimer>(
            timeval { heartSecond_, 0L }, this);
        shynet::utils::Singleton<net::TimerReactorMgr>::instance().add(pt);
        ping_timer_ = pt;
    }
}
net::InputResult Connector::input(std::function<void(std::unique_ptr<char[]>, size_t)> cb)
{
    //有数据接收到，因此延迟发送心跳计时器时间
    std::shared_ptr<PingTimer> pt = ping_timer_.lock();
    if (pt != nullptr) {
        pt->set_val({ heartSecond_, 0L });
    }
    return net::ConnectEvent::input(cb);
}

int Connector::message_handle(char* original_data, size_t datalen)
{
    return FilterData::message_handle(original_data, datalen);
}

void Connector::close(net::CloseType active)
{
    active_ = active;
    shynet::utils::Singleton<net::ConnectReactorMgr>::instance().remove(connectid());
}
void Connector::timerout(net::CloseType active)
{
    LOG_INFO << "检测到与服务器没有心跳超时 [ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
    close(active);
}

net::CloseType Connector::active() const
{
    return active_;
}
}
