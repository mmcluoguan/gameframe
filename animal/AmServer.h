#pragma once
#include "shynet/net/ListenReactorMgr.h"

namespace animal {
	class AmServer final : public net::ListenEvent {
	public:
		explicit AmServer(std::shared_ptr<net::IPAddress> listen_addr);
		~AmServer();

		std::weak_ptr<net::AcceptNewFd> accept_newfd(
			std::shared_ptr<net::IPAddress> remoteAddr,
			std::shared_ptr<events::EventBuffer> iobuf) override;
	private:

	};
}
