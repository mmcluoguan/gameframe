#include "game/gameserver.h"
#include "game/gameclient.h"
#include "game/gameclientmgr.h"
#include "shynet/net/timerreactormgr.h"

namespace game {
GameServer::GameServer(std::shared_ptr<net::IPAddress> listen_addr)
    : net::ListenEvent(listen_addr)
{
    LOG_INFO << "服务器game启动 [ip:" << listen_addr->ip() << ":" << listen_addr->port() << "]";
    shynet::utils::Singleton<GameClientMgr>::instance().set_listen_addr(*listenaddr());
}

GameServer::~GameServer()
{
    LOG_INFO << "服务器game关闭 [ip:" << listenaddr()->ip() << ":" << listenaddr()->port() << "]";
}

std::weak_ptr<net::AcceptNewFd> GameServer::accept_newfd(std::shared_ptr<net::IPAddress> remote_addr,
    std::shared_ptr<events::EventBuffer> iobuf)
{
    std::shared_ptr<GameClient> cli(new GameClient(remote_addr, listenaddr(), iobuf));
    shynet::utils::Singleton<GameClientMgr>::instance().add(iobuf->fd(), cli);
    return cli;
}
}
