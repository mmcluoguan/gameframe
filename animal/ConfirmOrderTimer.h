#pragma once
#include "shynet/net/IPAddress.h"
#include "shynet/net/TimerEvent.h"

namespace animal {
	class ConfirmOrderTimer : public net::TimerEvent {
	public:
		ConfirmOrderTimer(const struct timeval val);
		~ConfirmOrderTimer();

		void timeout() override;
	private:
	};
}
