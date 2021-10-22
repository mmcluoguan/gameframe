#include "shynet/net/connectevent.h"
#include "shynet/utils/logger.h"

namespace shynet {
extern pthread_barrier_t g_barrier;

namespace net {

    ConnectEvent::ConnectEvent(std::shared_ptr<net::IPAddress> connect_addr,
        FilterProces::ProtoType pt,
        bool enable_ssl,
        bool enableHeart, ssize_t heartSecond)
        : protocol::FilterProces(nullptr, pt, protocol::FilterProces::Identity::CONNECTOR)
    {
        connect_addr_ = connect_addr;
        enable_ssl_ = enable_ssl;
        heart_second_ = heartSecond;
        enable_heart_ = enableHeart;
        if (enable_ssl_) {
            ctx_ = SSL_CTX_new(SSLv23_client_method());
            if (ctx_ == nullptr) {
                THROW_EXCEPTION("call SSL_CTX_new");
            }
        }
    }

    ConnectEvent::ConnectEvent(const char* hostname, short port,
        FilterProces::ProtoType pt,
        bool enable_ssl, bool enableHeart, ssize_t heartSecond)
        : protocol::FilterProces(nullptr, pt, protocol::FilterProces::Identity::CONNECTOR)
    {
        enable_ssl_ = enable_ssl;
        heart_second_ = heartSecond;
        hostname_ = hostname;
        dnsport_ = port;
        enable_heart_ = enableHeart;
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

    int ConnectEvent::input()
    {
        return process();
    }

    bool ConnectEvent::enable_heart() const
    {
        return enable_heart_;
    }
    std::weak_ptr<ConnectHeartbeat> ConnectEvent::heart() const
    {
        return heart_;
    }

    void ConnectEvent::set_heart(std::weak_ptr<ConnectHeartbeat> ht)
    {
        heart_ = ht;
    }

    ssize_t ConnectEvent::heart_second() const
    {
        return heart_second_;
    }

    bool ConnectEvent::enable_dns() const
    {
        return enable_dns_;
    }

    short ConnectEvent::dnsport() const
    {
        return dnsport_;
    }

    void ConnectEvent::dnsbase(evdns_base* base)
    {
        dnsbase_ = base;
    }

    evdns_base* ConnectEvent::set_dnsbase() const
    {
        return dnsbase_;
    }

    std::string ConnectEvent::hostname() const
    {
        return hostname_;
    }
}
}
