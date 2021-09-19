#ifndef WORLD_HTTPSERVER_H
#define WORLD_HTTPSERVER_H

#include "shynet/net/ListenReactorMgr.h"

namespace world {
	/// <summary>
	/// http后台服务器
	/// </summary>
	class HttpServer final : public net::ListenEvent {
	public:
		explicit HttpServer(std::shared_ptr<net::IPAddress> listen_addr);
		~HttpServer();

		/// <summary>
		/// 接收新的连接
		/// </summary>
		/// <param name="remoteAddr">新连接的地址</param>
		/// <param name="iobuf">新连接读写缓冲区</param>
		/// <returns>新的连接</returns>
		std::weak_ptr<net::AcceptNewFd> accept_newfd(std::shared_ptr<net::IPAddress> remoteAddr,
			std::shared_ptr<events::EventBuffer> iobuf) override;
	private:

	};
}

#endif
