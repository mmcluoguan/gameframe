#ifndef FRMPUB_RECONNECTTIMER_H
#define FRMPUB_RECONNECTTIMER_H

#include "shynet/net/ConnectReactorMgr.h"
#include "shynet/net/IPAddress.h"
#include "shynet/net/TimerReactorMgr.h"

namespace frmpub {
	template <class T>
	class ReConnectTimer : public net::TimerEvent {
	public:
		ReConnectTimer(std::shared_ptr<net::IPAddress> connect_addr, const struct timeval val) :
			net::TimerEvent(val, EV_TIMEOUT) {
			connect_addr_ = connect_addr;
		}
		~ReConnectTimer() {
		}

		void timeout() override {
			shynet::utils::Singleton<net::TimerReactorMgr>::instance().remove(timerid());
			std::shared_ptr<T> reconnect(new T(connect_addr_));
			shynet::utils::Singleton<net::ConnectReactorMgr>::instance().add(reconnect);
			reconnect.reset();
		}
	private:
		std::shared_ptr<net::IPAddress> connect_addr_;
	};
}

#endif
