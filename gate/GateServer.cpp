#include "gate/GateServer.h"
#include "shynet/net/TimerReactorMgr.h"
#include "gate/GateClient.h"
#include "gate/GateClientMgr.h"

namespace gate
{
	GateServer::GateServer(std::shared_ptr<net::IPAddress> listen_addr) :
		net::ListenEvent(listen_addr, true)
	{
		LOG_INFO << "服务器gate启动 [ip:" << listen_addr->ip() << ":" << listen_addr->port() << "]";
<<<<<<< HEAD
		shynet::utils::Singleton<GateClientMgr>::instance().set_listen_addr(*listenaddr());
=======
		shynet::utils::Singleton<GateClientMgr>::instance().listen_addr(*listenaddr());
>>>>>>> 97f5d8ccc1392d6c54dfc663b535a803fe2f1f9e
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
