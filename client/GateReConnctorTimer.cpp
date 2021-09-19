#include "client/GateReConnctorTimer.h"
#include "shynet/net/ConnectReactorMgr.h"
#include "shynet/net/IPAddress.h"
#include "shynet/net/TimerReactorMgr.h"

extern int g_gateconnect_id;

namespace client {
	GateReConnctorTimer::GateReConnctorTimer(
		std::shared_ptr<net::IPAddress> connect_addr,
		std::shared_ptr<GateConnector::DisConnectData> data,
		const struct timeval val) :
		net::TimerEvent(val, EV_TIMEOUT | EV_PERSIST) {
		connect_addr_ = connect_addr;
		data_ = data;
	}

	GateReConnctorTimer::~GateReConnctorTimer() {
	}

	void GateReConnctorTimer::timeout() {
		shynet::utils::Singleton<net::TimerReactorMgr>::instance().remove(timerid());
		std::shared_ptr<GateConnector> reconnect(new GateConnector(connect_addr_, data_));
		g_gateconnect_id = shynet::utils::Singleton<net::ConnectReactorMgr>::instance().add(reconnect);
	}
}
