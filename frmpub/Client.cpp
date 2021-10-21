#include "frmpub/Client.h"
#include "shynet/utils/Logger.h"

namespace frmpub {
Client::Client(std::shared_ptr<net::IPAddress> remote_addr,
    std::shared_ptr<net::IPAddress> listen_addr,
    std::shared_ptr<events::EventBuffer> iobuf,
    bool enableHeart, ssize_t heartSecond,
    protocol::FilterProces::ProtoType pt,
    FilterData::ProtoData pd)
    : net::AcceptNewFd(remote_addr, listen_addr, iobuf, pt, enableHeart, heartSecond)
    , FilterData(pd)
{
    filter_ = this;
}

Client::~Client()
{
}

void Client::close(bool active)
{
    active_ = active;
}

void Client::timerout()
{
    LOG_INFO << "客户端心跳超时 [ip:" << remote_addr()->ip() << ":" << remote_addr()->port() << "]";
    close(true);
}

int Client::message_handle(char* original_data, size_t datalen)
{
    return FilterData::message_handle(original_data, datalen);
}

bool Client::active() const
{
    return active_;
}

protocc::ServerInfo Client::sif() const
{
    return sif_;
}
void Client::set_sif(const protocc::ServerInfo& v)
{
    sif_ = v;
}
}
