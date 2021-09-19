#pragma once
#include "shynet/events/EventBuffer.h"
#include <event2/bufferevent_ssl.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

namespace shynet {
	namespace events {
		class EventBufferSsl final : public EventBuffer {
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
