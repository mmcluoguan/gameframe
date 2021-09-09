#pragma once
#include "shynet/net/ListenReactorMgr.h"

namespace backend {
	class BkServer final : public net::ListenEvent {
	public:
		explicit BkServer(std::shared_ptr<net::IPAddress> listen_addr);
		~BkServer();

		std::weak_ptr<net::AcceptNewFd> accept_newfd(
			std::shared_ptr<net::IPAddress> remoteAddr,
			std::shared_ptr<events::EventBuffer> iobuf) override;
	private:

	};
}
