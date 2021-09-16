#include "shynet/events/EventBufferSsl.h"
#include "shynet/utils/Logger.h"

namespace shynet
{
	namespace events
	{
		EventBufferSsl::EventBufferSsl(std::shared_ptr<EventBase> base,
			evutil_socket_t fd, bufferevent_ssl_state state, int options, SSL_CTX* ctx) : EventBuffer(base)
		{
			ssl_ = SSL_new(ctx);
			if (ssl_ == nullptr) {
				throw SHYNETEXCEPTION("call SSL_new");
			}
			bufferevent* buf = bufferevent_openssl_socket_new(base->base(), fd, ssl_, state, options);
			if (buf == nullptr) {
				throw SHYNETEXCEPTION("call bufferevent_openssl_socket_new");
			}
			set_buffer(buf);
		}
		EventBufferSsl::~EventBufferSsl()
		{
			if (ssl_ != nullptr)
			{
				SSL_shutdown(ssl_);
				SSL_set_shutdown(ssl_, SSL_RECEIVED_SHUTDOWN);
				//SSL_free(ssl_);
			}
			if (buffer() != nullptr)
				bufferevent_free(buffer());
		}
	}
}
