#include "animal/AmServer.h"
#include "shynet/net/TimerReactorMgr.h"
#include "animal/AmClient.h"
#include "animal/AmClientMgr.h"
#include "animal/ConfirmOrderTimer.h"

namespace animal {
	AmServer::AmServer(std::shared_ptr<net::IPAddress> listen_addr) :
		net::ListenEvent(listen_addr) {
		GOBJ++;
		LOG_TRACE << "AmServer:" << GOBJ;
		LOG_INFO << "服务器animal启动 [ip:" << listen_addr->ip() << ":" << listen_addr->port() << "]";
		shynet::Singleton<AmClientMgr>::instance().listen_addr(*listenaddr());

		//30分定时确认订单
		std::shared_ptr<ConfirmOrderTimer> cot(new ConfirmOrderTimer({ 60L * 30L,0L }));
		shynet::Singleton<net::TimerReactorMgr>::instance().add(cot);
	}

	AmServer::~AmServer() {
		GOBJ--;
		LOG_TRACE << "~AmServer:" << GOBJ;
		LOG_INFO << "服务器animal关闭 [ip:" << listenaddr()->ip() << ":" << listenaddr()->port() << "]";
	}

	std::weak_ptr<net::AcceptNewFd> AmServer::accept_newfd(
		std::shared_ptr<net::IPAddress> remote_addr,
		std::shared_ptr<events::EventBuffer> iobuf) {
		std::shared_ptr<AmClient> cli(new AmClient(remote_addr, listenaddr(), iobuf));
		shynet::Singleton<AmClientMgr>::instance().add(iobuf->fd(), cli);

		return cli;
	}
}
