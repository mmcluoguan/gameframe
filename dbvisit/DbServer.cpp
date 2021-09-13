#include "dbvisit/DbServer.h"
#include "shynet/net/TimerReactorMgr.h"
#include "shynet/lua/LuaEngine.h"
#include "dbvisit/DbClient.h"
#include "dbvisit/DbClientMgr.h"

namespace dbvisit
{
	DbServer::DbServer(std::shared_ptr<net::IPAddress> listen_addr) :
		net::ListenEvent(listen_addr)
	{
		LOG_INFO << "服务器dbvisit启动 [ip:" << listen_addr->ip() << ":" << listen_addr->port() << "]";
		shynet::utils::Singleton<DbClientMgr>::instance().set_listen_addr(*listenaddr());
	}

	DbServer::~DbServer()
	{
		LOG_INFO << "服务器dbvisit关闭 [ip:" << listenaddr()->ip() << ":" << listenaddr()->port() << "]";
	}

	std::weak_ptr<net::AcceptNewFd> DbServer::accept_newfd(
		std::shared_ptr<net::IPAddress> remote_addr,
		std::shared_ptr<events::EventBuffer> iobuf)
	{
		std::shared_ptr<DbClient> cli(new DbClient(remote_addr, listenaddr(), iobuf));
		shynet::utils::Singleton<DbClientMgr>::instance().add(iobuf->fd(), cli);

		return cli;
	}
}
