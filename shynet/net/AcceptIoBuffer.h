#ifndef SHYNET_NET_ACCEPTIOBUFFER_H
#define SHYNET_NET_ACCEPTIOBUFFER_H

#include "shynet/events/EventBufferSsl.h"
#include "shynet/net/AcceptNewFd.h"

namespace shynet {
namespace net {
    class AcceptIoBuffer : public events::EventBuffer {
    public:
        AcceptIoBuffer(std::shared_ptr<events::EventBase> base,
            evutil_socket_t fd, bool enable_ssl = false, SSL_CTX* ctx = nullptr);
        ~AcceptIoBuffer();

        std::weak_ptr<AcceptNewFd> newfd() const;
        void set_newfd(std::weak_ptr<AcceptNewFd> newfd);

        void io_readcb();
        void io_writecb();
        void io_eventcb(short events);

    private:
        std::shared_ptr<events::EventBuffer> iobuf_;
        std::weak_ptr<AcceptNewFd> newfd_;
    };
}
}

#endif
