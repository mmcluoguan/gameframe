#pragma once
#include "shynet/events/EventBufferSsl.h"
#include "shynet/net/ConnectEvent.h"

namespace shynet
{
	namespace net
	{
		class ConnectIoBuffer : public events::EventBuffer
		{
		public:
			ConnectIoBuffer(std::shared_ptr<events::EventBase> base, bool enable_ssl = false, SSL_CTX* ctx = nullptr);
			~ConnectIoBuffer();

			std::weak_ptr<ConnectEvent> cnev() const;
			void cnev(std::weak_ptr<ConnectEvent> newfd);

			void io_readcb();
			void io_writecb();
			void io_eventcb(short events);
		private:
			std::shared_ptr<events::EventBuffer> iobuf_;
			std::weak_ptr<ConnectEvent> cnev_;
		};
	}
}
