#include "world/worldserver.h"
#include "shynet/net/timerreactormgr.h"
#include "world/worldclient.h"
#include "world/worldclientmgr.h"

namespace world {
WorldServer::WorldServer(std::shared_ptr<net::IPAddress> listen_addr)
    : net::ListenEvent(listen_addr, SOCK_DGRAM)
{
    LOG_INFO << "服务器world启动 [ip:" << listen_addr->ip() << ":" << listen_addr->port() << "]";
    shynet::utils::Singleton<WorldClientMgr>::instance().set_listen_addr(*listenaddr());
}

WorldServer::~WorldServer()
{
    LOG_INFO << "服务器world关闭 [ip:" << listenaddr()->ip() << ":" << listenaddr()->port() << "]";
}

std::weak_ptr<net::AcceptNewFd> WorldServer::accept_newfd(
    std::shared_ptr<net::IPAddress> remote_addr,
    std::shared_ptr<events::EventBuffer> iobuf)
{
    std::shared_ptr<WorldClient> cli(new WorldClient(remote_addr, listenaddr(), iobuf));
    shynet::utils::Singleton<WorldClientMgr>::instance().add(iobuf->fd(), cli);
    return cli;
}
}
