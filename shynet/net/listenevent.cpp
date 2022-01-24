#include "shynet/net/listenevent.h"
#include "shynet/net/acceptreactormgr.h"
#include "shynet/net/listenreactormgr.h"
#include <netinet/tcp.h>

namespace shynet {
namespace net {
    ListenEvent::ListenEvent(std::shared_ptr<net::IPAddress> listen_addr,
        __socket_type type,
        bool enable_ssl)
        : events::EventHandler()
    {
        self = this;
        listen_addr_ = listen_addr;
        enable_ssl_ = enable_ssl;
        type_ = type;
        if (type_ == SOCK_STREAM) {
            init_tcp();
        } else if (type_ == SOCK_DGRAM) {
            init_udp();
        } else {
            THROW_EXCEPTION("不支持的socket_type:" + std::to_string(type_));
        }
    }
    ListenEvent::~ListenEvent()
    {
        if (ctx_ != nullptr)
            SSL_CTX_free(ctx_);
    }

    void ListenEvent::input(int listenfd)
    {
        utils::Singleton<AcceptReactorMgr>::instance().notify(&self, sizeof(uintptr_t));
    }

    bool ListenEvent::stop() const
    {
        return utils::Singleton<ListenReactorMgr>::instance().remove(serverid_);
    }

    void ListenEvent::pause()
    {
        if (ispause_ == false) {
            base()->delevent(this);
            ispause_ = true;
        }
    }

    void ListenEvent::resume()
    {
        if (ispause_) {
            base()->addevent(this, nullptr);
            ispause_ = false;
        }
    }

    void ListenEvent::init_tcp()
    {
        if (enable_ssl_ == true) {
            ctx_ = SSL_CTX_new(SSLv23_server_method());
            if (ctx_ == nullptr) {
                THROW_EXCEPTION("call SSL_new");
            }
            if (!SSL_CTX_use_certificate_chain_file(ctx_, "cert") || !SSL_CTX_use_PrivateKey_file(ctx_, "pkey", SSL_FILETYPE_PEM)) {
                const char* str = "Couldn't read 'pkey' or'cert' file.  To generate a key\n"
                                  "and self-signed certificate,run:\n"
                                  "  openssl genrsa -out pkey 2048\n"
                                  "  openssl req -new -key pkey -out cert.req\n"
                                  "  openssl x509 -req -days 365 -in cert.req -signkey pkey -out cert \n\t -";
                THROW_EXCEPTION(str);
            }
        }

        listenfd_ = socket(listen_addr_->family(), SOCK_STREAM, IPPROTO_IP);
        if (listenfd_ == -1) {
            THROW_EXCEPTION("call socket");
        }

        if (evutil_make_socket_nonblocking(listenfd_) < 0) {
            evutil_closesocket(listenfd_);
            THROW_EXCEPTION("call evutil_make_socket_nonblocking");
        }
        if (evutil_make_socket_closeonexec(listenfd_) < 0) {
            evutil_closesocket(listenfd_);
            THROW_EXCEPTION("call evutil_make_socket_closeonexec");
        }
        int on = 1;
        if (setsockopt(listenfd_, SOL_SOCKET, SO_KEEPALIVE, (void*)&on, sizeof(on)) < 0) {
            evutil_closesocket(listenfd_);
            THROW_EXCEPTION("call setsockopt");
        }
        if (setsockopt(listenfd_, SOL_TCP, TCP_NODELAY, (void*)&on, sizeof(on)) != 0) {
            evutil_closesocket(listenfd_);
            THROW_EXCEPTION("call setsockopt");
        }
        if (evutil_make_listen_socket_reuseable(listenfd_) < 0) {
            evutil_closesocket(listenfd_);
            THROW_EXCEPTION("call evutil_make_listen_socket_reuseable");
        }

        if (bind(listenfd_, (const struct sockaddr*)listen_addr_->sockaddr(), sizeof(sockaddr_storage)) < 0) {
            evutil_closesocket(listenfd_);
            std::ostringstream err;
            err << "call bind:" << EVUTIL_SOCKET_ERROR() << " " << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()) << " -";
            THROW_EXCEPTION(err.str());
        }

        if (listen(listenfd_, 128) < 0) {
            evutil_closesocket(listenfd_);
            THROW_EXCEPTION("call listen");
        }
        LOG_TRACE << "tcp listen ip:" << listen_addr_->ip() << " port:" << listen_addr_->port();
    }

    void ListenEvent::init_udp()
    {
        listenfd_ = socket(listen_addr_->family(), SOCK_DGRAM, IPPROTO_IP);
        if (listenfd_ == -1) {
            THROW_EXCEPTION("call socket");
        }

        if (evutil_make_socket_nonblocking(listenfd_) < 0) {
            evutil_closesocket(listenfd_);
            THROW_EXCEPTION("call evutil_make_socket_nonblocking");
        }
        if (evutil_make_socket_closeonexec(listenfd_) < 0) {
            evutil_closesocket(listenfd_);
            THROW_EXCEPTION("call evutil_make_socket_closeonexec");
        }

        if (evutil_make_listen_socket_reuseable(listenfd_) < 0) {
            evutil_closesocket(listenfd_);
            THROW_EXCEPTION("call evutil_make_listen_socket_reuseable");
        }

        if (bind(listenfd_, (const struct sockaddr*)listen_addr_->sockaddr(), sizeof(sockaddr_storage)) < 0) {
            evutil_closesocket(listenfd_);
            std::ostringstream err;
            err << "call bind:" << EVUTIL_SOCKET_ERROR() << " " << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()) << " -";
            THROW_EXCEPTION(err.str());
        }
        LOG_TRACE << "udp bind ip:" << listen_addr_->ip() << " port:" << listen_addr_->port();
    }
}
}
