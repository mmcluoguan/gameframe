#include "log/logserver.h"
#include "log/logclient.h"
#include "log/logclientmgr.h"

namespace logs {
LogServer::LogServer(std::shared_ptr<net::IPAddress> listen_addr)
    : net::ListenEvent(listen_addr)
{
    LOG_INFO << "服务器log启动 [ip:" << listen_addr->ip() << ":" << listen_addr->port() << "]";
    shynet::utils::Singleton<LogClientMgr>::instance().set_listen_addr(*listenaddr());
}

LogServer::~LogServer()
{
    LOG_INFO << "服务器log关闭 [ip:" << listenaddr()->ip() << ":" << listenaddr()->port() << "]";
}

std::weak_ptr<net::AcceptNewFd> LogServer::accept_newfd(
    std::shared_ptr<net::IPAddress> remote_addr,
    std::shared_ptr<events::EventBuffer> iobuf)
{
    std::shared_ptr<LogClient> cli(new LogClient(remote_addr, listenaddr(), iobuf));
    shynet::utils::Singleton<LogClientMgr>::instance().add(iobuf->fd(), cli);

    return cli;
}
}
