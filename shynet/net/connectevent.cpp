#include "shynet/net/connectevent.h"
#include "shynet/pool/threadpool.h"
#include "shynet/utils/logger.h"
#include "shynet/utils/singleton.h"
#include "shynet/utils/stuff.h"

namespace shynet {
namespace net {

    ConnectEvent::ConnectEvent(std::shared_ptr<net::IPAddress> connect_addr,
        FilterProces::ProtoType pt,
        __socket_type type,
        bool enable_ssl,
        bool enable_check, ssize_t check_second)
        : protocol::FilterProces(nullptr, pt, protocol::FilterProces::Identity::CONNECTOR)
    {
        connect_addr_ = connect_addr;
        type_ = type;
        enable_ssl_ = enable_ssl;
        check_second_ = check_second;
        enable_check_ = enable_check;
        if (enable_ssl_ && type_ == SOCK_STREAM) {
            ctx_ = SSL_CTX_new(SSLv23_client_method());
            if (ctx_ == nullptr) {
                THROW_EXCEPTION("call SSL_CTX_new");
            }
        }
    }

    ConnectEvent::ConnectEvent(const char* hostname, short port,
        FilterProces::ProtoType pt,
        __socket_type type,
        bool enable_ssl, bool enable_check, ssize_t check_second)
        : protocol::FilterProces(nullptr, pt, protocol::FilterProces::Identity::CONNECTOR)
    {
        hostname_ = hostname;
        dnsport_ = port;
        type_ = type;
        enable_ssl_ = enable_ssl;
        enable_check_ = enable_check;
        check_second_ = check_second;
        struct hostent* phe = gethostbyname(hostname);
        if (phe == 0 || phe->h_addr_list[0] == 0) {
            THROW_EXCEPTION("call gethostbyname");
        }
        struct in_addr addr;
        memset(&addr, 0, sizeof(in_addr));
        memcpy(&addr, phe->h_addr_list[0], sizeof(struct in_addr));
        char ip[65] = { 0 };
        strcpy(ip, inet_ntoa(addr));
        connect_addr_ = std::make_shared<IPAddress>(ip, port);
        if (enable_ssl_ && type_ == SOCK_STREAM) {
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
        return connectid_;
    }

    void ConnectEvent::set_connectid(int id)
    {
        connectid_ = id;
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

    void ConnectEvent::input(int fd)
    {
        if (type_ == SOCK_DGRAM) {
            auto udpth = utils::Singleton<pool::ThreadPool>::instance().udpTh().lock();
            if (udpth) {
                int ident = static_cast<int>(protocol::FilterProces::Identity::CONNECTOR);
                udpth->notify(ident, connectid_);
            } else {
                THROW_EXCEPTION("udpthread 未创建");
            }
        }
    }

    InputResult ConnectEvent::input(std::function<void(std::unique_ptr<char[]>, size_t)> cb)
    {
        return process(cb);
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

    uint32_t ConnectEvent::fd() const
    {
        if (type_ == SOCK_STREAM) {
            return iobuf()->fd();
        } else {
            return udpsocket_->guid();
        }
    }
}
}
