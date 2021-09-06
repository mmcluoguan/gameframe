#include "game/GameServer.h"
#include "shynet/net/TimerReactorMgr.h"
#include "game/GameClient.h"
#include "game/GameClientMgr.h"

namespace game {
	GameServer::GameServer(std::shared_ptr<net::IPAddress> listen_addr) :
		net::ListenEvent(listen_addr) {
		LOG_INFO << "服务器game启动 [ip:" << listen_addr->ip() << ":" << listen_addr->port() << "]";
		shynet::Singleton<GameClientMgr>::instance().listen_addr(*listenaddr());
	}

	GameServer::~GameServer() {
		LOG_INFO << "服务器game关闭 [ip:" << listenaddr()->ip() << ":" << listenaddr()->port() << "]";
	}

	std::weak_ptr<net::AcceptNewFd> GameServer::accept_newfd(std::shared_ptr<net::IPAddress> remote_addr,
		std::shared_ptr<events::EventBuffer> iobuf) {
		std::shared_ptr<GameClient> cli(new GameClient(remote_addr, listenaddr(), iobuf));
		shynet::Singleton<GameClientMgr>::instance().add(iobuf->fd(), cli);
		return cli;
	}
}
