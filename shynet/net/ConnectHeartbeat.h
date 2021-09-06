#pragma once
#include "shynet/net/TimerEvent.h"

namespace shynet
{
	namespace net
	{
		class ConnectEvent;
		class ConnectHeartbeat : public TimerEvent
		{
		public:
			ConnectHeartbeat(std::weak_ptr<ConnectEvent> cnv, const struct timeval val);
			~ConnectHeartbeat();

			void timeout();
		private:
			std::weak_ptr<ConnectEvent> cnv_;
		};

	}
}
