#include "shynet/events/eventbase.h"
#include "3rd/jemalloc/jemalloc.h"
#include "shynet/events/eventbuffer.h"
#include "shynet/events/eventhandler.h"
#include "shynet/utils/logger.h"
#include <event2/bufferevent.h>
#include <event2/event-config.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

namespace shynet {
namespace events {
    EventBase::EventBase()
    {
        const EventConfig cfg;
        if (cfg.setfeatures(EV_FEATURE_ET) == -1) {
            THROW_EXCEPTION("call setfeatures(EV_FEATURE_ET)");
        }
        if (cfg.setfeatures(EV_FEATURE_O1) == -1) {
            THROW_EXCEPTION("call setfeatures(EV_FEATURE_O1)");
        }
        if (cfg.setflag(EVENT_BASE_FLAG_IGNORE_ENV) == -1) {
            THROW_EXCEPTION("call setfeatures(EVENT_BASE_FLAG_IGNORE_ENV)");
        }
        if (cfg.setflag(EVENT_BASE_FLAG_EPOLL_USE_CHANGELIST) == -1) {
            THROW_EXCEPTION("call setflag(EVENT_BASE_FLAG_EPOLL_USE_CHANGELIST)");
        }
        base_ = event_base_new_with_config(cfg.config());
        if (base_ == nullptr) {
            THROW_EXCEPTION("call event_base_new_with_config");
        }
    }
    EventBase::EventBase(const EventConfig* cfg)
    {
        base_ = event_base_new_with_config(cfg->config());
        if (base_ == nullptr) {
            THROW_EXCEPTION("call event_base_new_with_config");
        }
    }
    EventBase::~EventBase()
    {
        if (base_ != nullptr)
            event_base_free(base_);
    }
    event_base* EventBase::base() const
    {
        return base_;
    }
    int EventBase::dispatch() const
    {
        return event_base_dispatch(base_);
    }
    int EventBase::loop(int flag) const
    {
        event_base_loop(base_, flag);
        return 0;
    }
    int EventBase::loopexit(const timeval* tv) const
    {
        return event_base_loopexit(base_, tv);
    }
    int EventBase::loopbreak() const
    {
        return event_base_loopbreak(base_);
    }
    bool EventBase::gotexit() const
    {
        return event_base_got_exit(base_);
    }
    bool EventBase::gotbreak() const
    {
        return event_base_got_break(base_);
    }
    int EventBase::addevent(const std::shared_ptr<EventHandler> handler, const timeval* tv) const
    {
        return addevent(handler.get(), tv);
    }
    int EventBase::addevent(const EventHandler* handler, const timeval* tv) const
    {
        return event_add(handler->event(), tv);
    }
    int EventBase::delevent(const std::shared_ptr<EventHandler> handler) const
    {
        return delevent(handler.get());
    }
    int EventBase::delevent(const EventHandler* handler) const
    {
        return event_del(handler->event());
    }
    int EventBase::usethread()
    {
        event_set_mem_functions(je_malloc, je_realloc, je_free);
        return evthread_use_pthreads();
    }
    void EventBase::event_shutdown()
    {
        libevent_global_shutdown();
    }
    void EventBase::initssl()
    {
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        ERR_load_crypto_strings();
        SSL_load_error_strings();
    }
    void EventBase::cleanssl()
    {
        FIPS_mode_set(0);
        EVP_cleanup();
        CRYPTO_cleanup_all_ex_data();
        ERR_remove_state(0);
        ERR_free_strings();
        SSL_COMP_free_compression_methods();
    }
    void EventBase::make_pair_buffer(std::shared_ptr<EventBuffer> pair[2])
    {
        if (pair[0] != nullptr || pair[1] != nullptr) {
            THROW_EXCEPTION("call make_pair_buffer");
        }
        struct bufferevent* ap[2] = { nullptr };
        if (bufferevent_pair_new(base_, BEV_OPT_THREADSAFE | BEV_OPT_CLOSE_ON_FREE, ap) != 0) {
            THROW_EXCEPTION("call bufferevent_pair_new");
        }
        pair[0] = std::make_shared<EventBuffer>(ap[0], true);
        pair[1] = std::make_shared<EventBuffer>(ap[1], true);
    }
}
}
