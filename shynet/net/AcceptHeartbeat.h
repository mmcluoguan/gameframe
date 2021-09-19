#ifndef SHYNET_NET_ACCEPTHEARTBEAT_H
#define SHYNET_NET_ACCEPTHEARTBEAT_H

#include "shynet/net/TimerReactorMgr.h"

namespace shynet {
	namespace net {
		class AcceptNewFd;
		class AcceptHeartbeat : public TimerEvent {
		public:
			AcceptHeartbeat(std::weak_ptr<AcceptNewFd> newfd, const struct timeval val);
			~AcceptHeartbeat();

			/// <summary>
			/// 计时器线程超时回调
			/// </summary>
			void timeout() override;
		private:
			std::weak_ptr<AcceptNewFd> newfd_;
		};


	}
}

#endif
