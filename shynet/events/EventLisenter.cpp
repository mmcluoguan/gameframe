#include "shynet/events/eventlisenter.h"
#include <cstring>

#include "shynet/utils/logger.h"

namespace shynet {
namespace events {
    static void lcb(struct evconnlistener* listener,
        evutil_socket_t sock,
        struct sockaddr* addr,
        int len,
        void* ptr)
    {
        EventLisenter* ls = static_cast<EventLisenter*>(ptr);
        ls->input(sock, addr, len);
    }

    static void acb(struct evconnlistener* listener, void* ctx)
    {
        LOG_WARN << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR());
        EventLisenter* ls = static_cast<EventLisenter*>(ctx);
        ls->error();
    }

    EventLisenter::EventLisenter(std::shared_ptr<EventBase> base, const sockaddr* sa, int socklen)
    {
        base_ = base;
        listener_ = evconnlistener_new_bind(base->base(), lcb, this,
            LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE | LEV_OPT_THREADSAFE,
            -1, sa, socklen);
        if (listener_ == nullptr) {
            THROW_EXCEPTION("call evconnlistener_new_bind");
        }
        evconnlistener_set_error_cb(listener_, acb);
    }
    EventLisenter::~EventLisenter()
    {
        if (listener_ != nullptr)
            evconnlistener_free(listener_);
    }
    std::shared_ptr<EventBase> EventLisenter::base() const
    {
        return base_;
    }
    evutil_socket_t EventLisenter::fd() const
    {
        return evconnlistener_get_fd(listener_);
    }
    int EventLisenter::disable() const
    {
        return evconnlistener_disable(listener_);
    }
    int EventLisenter::enable() const
    {
        return evconnlistener_enable(listener_);
    }
}
}
