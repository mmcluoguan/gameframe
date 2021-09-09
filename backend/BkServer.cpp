#include "backend/BkServer.h"
#include "shynet/net/TimerReactorMgr.h"
#include "backend/BkClient.h"
#include "backend/BkClientMgr.h"

namespace backend {
	BkServer::BkServer(std::shared_ptr<net::IPAddress> listen_addr) :
		net::ListenEvent(listen_addr) {
		GOBJ++;
		LOG_TRACE << "BkServer:" << GOBJ;
		LOG_INFO << "服务器backend启动 [ip:" << listen_addr->ip() << ":" << listen_addr->port() << "]";
		shynet::Singleton<BkClientMgr>::instance().listen_addr(*listenaddr());
	}

	BkServer::~BkServer() {
		GOBJ--;
		LOG_TRACE << "~BkServer:" << GOBJ;
		LOG_INFO << "服务器backend关闭 [ip:" << listenaddr()->ip() << ":" << listenaddr()->port() << "]";
	}

	std::weak_ptr<net::AcceptNewFd> BkServer::accept_newfd(
		std::shared_ptr<net::IPAddress> remote_addr,
		std::shared_ptr<events::EventBuffer> iobuf) {
		std::shared_ptr<BkClient> cli(new BkClient(remote_addr, listenaddr(), iobuf));
		shynet::Singleton<BkClientMgr>::instance().add(iobuf->fd(), cli);

		return cli;
	}
}
