#pragma once
#include "client/GateConnector.h"
#include "shynet/net/IPAddress.h"
#include "shynet/net/TimerEvent.h"

namespace client {
	class GateReConnctorTimer : public net::TimerEvent {
	public:
		GateReConnctorTimer(
			std::shared_ptr<net::IPAddress> connect_addr,
			std::shared_ptr<GateConnector::DisConnectData> data,
			const struct timeval val
			);
		~GateReConnctorTimer();

		void timeout() override;
	private:
		std::shared_ptr<net::IPAddress> connect_addr_;
		std::shared_ptr<GateConnector::DisConnectData> data_;
	};
}
