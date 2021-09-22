#ifndef FRMPUB_PINGTIMER_H
#define FRMPUB_PINGTIMER_H

#include "shynet/net/IPAddress.h"
#include "shynet/net/TimerEvent.h"

namespace frmpub {
	class Connector;
	class PingTimer : public net::TimerEvent {
	public:
		PingTimer(const struct timeval val,
			Connector* connector);
		~PingTimer();

		void timeout() override;
		void clean_connector() {
			connector_ = nullptr;
		}
	private:
		Connector* connector_;
	};
}

#endif
