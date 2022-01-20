#include "shynet/events/eventbufferssl.h"
#include "shynet/utils/logger.h"
#include <csignal>

namespace shynet {
namespace events {

    EventBufferSsl::EventBufferSsl(std::shared_ptr<EventBase> base,
        evutil_socket_t fd, bufferevent_ssl_state state, int options, SSL_CTX* ctx)
        : EventBuffer(base)
    {
        ssl_ = SSL_new(ctx);
        if (ssl_ == nullptr) {
            THROW_EXCEPTION("call SSL_new");
        }
        bufferevent* buf = bufferevent_openssl_socket_new(base->base(), fd, ssl_, state, options);
        if (buf == nullptr) {
            THROW_EXCEPTION("call bufferevent_openssl_socket_new");
        }
        set_buffer(buf);
    }
    EventBufferSsl::~EventBufferSsl()
    {
        if (ssl_ != nullptr) {
            std::signal(SIGPIPE, SIG_IGN);
            SSL_set_shutdown(ssl_, SSL_RECEIVED_SHUTDOWN | SSL_SENT_SHUTDOWN);
            SSL_shutdown(ssl_);
            SSL_free(ssl_);
        }
        if (buffer() != nullptr)
            bufferevent_free(buffer());
    }
}
}
