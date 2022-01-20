#include "shynet/net/connectevent.h"
#include "shynet/utils/logger.h"

namespace shynet {
extern pthread_barrier_t g_barrier;

namespace net {

    ConnectEvent::ConnectEvent(std::shared_ptr<net::IPAddress> connect_addr,
        FilterProces::ProtoType pt,
        bool enable_ssl,
        bool enable_check, ssize_t check_second)
        : protocol::FilterProces(nullptr, pt, protocol::FilterProces::Identity::CONNECTOR)
    {
        connect_addr_ = connect_addr;
        enable_ssl_ = enable_ssl;
        check_second_ = check_second;
        enable_check_ = enable_check;
        if (enable_ssl_) {
            ctx_ = SSL_CTX_new(SSLv23_client_method());
            if (ctx_ == nullptr) {
                THROW_EXCEPTION("call SSL_CTX_new");
            }
        }
    }

    ConnectEvent::ConnectEvent(const char* hostname, short port,
        FilterProces::ProtoType pt,
        bool enable_ssl, bool enable_check, ssize_t check_second)
        : protocol::FilterProces(nullptr, pt, protocol::FilterProces::Identity::CONNECTOR)
    {
        hostname_ = hostname;
        dnsport_ = port;
        enable_ssl_ = enable_ssl;
        enable_check_ = enable_check;
        check_second_ = check_second;
        if (enable_ssl_) {
            ctx_ = SSL_CTX_new(SSLv23_client_method());
            if (ctx_ == nullptr) {
                THROW_EXCEPTION("call SSL_CTX_new");
            }
        }
    }

    ConnectEvent::~ConnectEvent()
    {
        if (dnsbase_ != nullptr) {
            evdns_base_free(dnsbase_, 0);
        }
        if (ctx_ != nullptr)
            SSL_CTX_free(ctx_);
    }

    int ConnectEvent::connectid() const
    {
        return conectid_;
    }

    void ConnectEvent::connectid(int id)
    {
        conectid_ = id;
    }

    std::shared_ptr<net::IPAddress> ConnectEvent::connect_addr() const
    {
        return connect_addr_;
    }

    bool ConnectEvent::enable_ssl() const
    {
        return enable_ssl_;
    }

    SSL_CTX* ConnectEvent::ctx() const
    {
        return ctx_;
    }

    void ConnectEvent::success()
    {
        if (pt() == ProtoType::WEBSOCKET) {
            request_handshake();
        } else
            complete();
    }

    InputResult ConnectEvent::input()
    {
        return process();
    }

    bool ConnectEvent::enable_check() const
    {
        return enable_check_;
    }
    int ConnectEvent::check_timeid() const
    {
        return check_timeid_;
    }

    void ConnectEvent::set_check_timeid(int ht)
    {
        check_timeid_ = ht;
    }

    ssize_t ConnectEvent::check_second() const
    {
        return check_second_;
    }
}
}
