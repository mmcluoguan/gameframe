#include "login/LoginServer.h"
#include "login/LoginClientMgr.h"
#include "login/LoginClient.h"
#include "shynet/net/TimerReactorMgr.h"

namespace login
{
	LoginServer::LoginServer(std::shared_ptr<net::IPAddress> listen_addr) :
		net::ListenEvent(listen_addr)
	{
		LOG_INFO << "服务器login启动 [ip:" << listen_addr->ip() << ":" << listen_addr->port() << "]";
		shynet::utils::Singleton<LoginClientMgr>::instance().set_listen_addr(*listenaddr());
	}

	LoginServer::~LoginServer()
	{
		LOG_INFO << "服务器login关闭 [ip:" << listenaddr()->ip() << ":" << listenaddr()->port() << "]";
	}

	std::weak_ptr<net::AcceptNewFd> LoginServer::accept_newfd(std::shared_ptr<net::IPAddress> remote_addr,
		std::shared_ptr<events::EventBuffer> iobuf)
	{
		std::shared_ptr<LoginClient> cli(new LoginClient(remote_addr, listenaddr(), iobuf));
		shynet::utils::Singleton<LoginClientMgr>::instance().add(iobuf->fd(), cli);
		return cli;
	}
}
