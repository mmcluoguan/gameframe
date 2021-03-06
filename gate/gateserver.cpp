#include "gate/gateserver.h"
#include "gate/gateclient.h"
#include "gate/gateclientmgr.h"
#include "shynet/net/timerreactormgr.h"

namespace gate {
GateServer::GateServer(std::shared_ptr<net::IPAddress> listen_addr)
    : net::ListenEvent(listen_addr, SOCK_DGRAM)
{
    LOG_INFO << "服务器gate启动 [ip:" << listen_addr->ip() << ":" << listen_addr->port() << "]";
    shynet::utils::Singleton<GateClientMgr>::instance().set_listen_addr(*listenaddr());
}

GateServer::~GateServer()
{
    LOG_INFO << "服务器gate关闭 [ip:" << listenaddr()->ip() << ":" << listenaddr()->port() << "]";
}

std::weak_ptr<net::AcceptNewFd> GateServer::accept_newfd(std::shared_ptr<net::IPAddress> remote_addr,
    std::shared_ptr<events::EventBuffer> iobuf)
{
    std::shared_ptr<GateClient> cli(new GateClient(remote_addr, listenaddr(), iobuf));
    shynet::utils::Singleton<GateClientMgr>::instance().add(iobuf->fd(), cli);
    return cli;
}
}
