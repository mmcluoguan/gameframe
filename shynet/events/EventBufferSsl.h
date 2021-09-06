#pragma once
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>
#include "shynet/events/EventBuffer.h"
#include "shynet/events/EventBase.h"

namespace shynet
{
	namespace events
	{
		class EventBufferSsl final : public EventBuffer
		{
		public:
			EventBufferSsl(std::shared_ptr<EventBase> base,
				evutil_socket_t fd,
				bufferevent_ssl_state state,
				int options,
				SSL_CTX* ctx);
			~EventBufferSsl();

		private:
			SSL* ssl_ = nullptr;
		};
	}
}
