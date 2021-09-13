#include "world/HttpServer.h"
#include "shynet/net/TimerReactorMgr.h"
#include "world/HttpClient.h"
#include "world/HttpClientMgr.h"

namespace world {
	HttpServer::HttpServer(std::shared_ptr<net::IPAddress> listen_addr) :
		net::ListenEvent(listen_addr, true) {
		LOG_INFO << "服务器http启动 [ip:" << listen_addr->ip() << ":" << listen_addr->port() << "]";
		shynet::utils::Singleton<HttpClientMgr>::instance().set_listen_addr(*listenaddr());
	}

	HttpServer::~HttpServer() {
		LOG_INFO << "服务器http关闭 [ip:" << listenaddr()->ip() << ":" << listenaddr()->port() << "]";
	}

	std::weak_ptr<net::AcceptNewFd> HttpServer::accept_newfd(
		std::shared_ptr<net::IPAddress> remote_addr,
		std::shared_ptr<events::EventBuffer> iobuf) {
		std::shared_ptr<HttpClient> cli(new HttpClient(remote_addr, listenaddr(), iobuf));
		shynet::utils::Singleton<HttpClientMgr>::instance().add(iobuf->fd(), cli);
		return cli;
	}
}
