#include "frmpub/client.h"
#include "shynet/utils/logger.h"

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

void Client::close(net::CloseType active)
{
    active_ = active;
}

void Client::timerout(net::CloseType active)
{
    LOG_INFO << "客户端心跳超时 [ip:" << remote_addr()->ip() << ":" << remote_addr()->port() << "]";
    close(active);
}

int Client::message_handle(char* original_data, size_t datalen)
{
    return FilterData::message_handle(original_data, datalen);
}

}
