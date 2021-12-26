#include "shynet/net/listenevent.h"
#include "shynet/net/acceptreactormgr.h"
#include "shynet/net/listenreactormgr.h"

namespace shynet {
namespace net {
    ListenEvent::ListenEvent(std::shared_ptr<net::IPAddress> listen_addr, bool enable_ssl)
        : events::EventHandler()
    {
        self = this;
        listen_addr_ = listen_addr;
        enable_ssl_ = enable_ssl;
        if (enable_ssl == true) {
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

        evutil_socket_t fd = socket(listen_addr_->family(), SOCK_STREAM, IPPROTO_IP);
        if (fd == -1) {
            THROW_EXCEPTION("call socket");
        }

        if (evutil_make_socket_nonblocking(fd) < 0) {
            evutil_closesocket(fd);
            THROW_EXCEPTION("call evutil_make_socket_nonblocking");
        }
        if (evutil_make_socket_closeonexec(fd) < 0) {
            evutil_closesocket(fd);
            THROW_EXCEPTION("call evutil_make_socket_closeonexec");
        }
        int on = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void*)&on, sizeof(on)) < 0) {
            evutil_closesocket(fd);
            THROW_EXCEPTION("call setsockopt");
        }
        if (evutil_make_listen_socket_reuseable(fd) < 0) {
            evutil_closesocket(fd);
            THROW_EXCEPTION("call evutil_make_listen_socket_reuseable");
        }

        if (bind(fd, (const struct sockaddr*)listen_addr_->sockaddr(), sizeof(sockaddr_storage)) < 0) {
            evutil_closesocket(fd);
            std::ostringstream err;
            err << "call bind:" << EVUTIL_SOCKET_ERROR() << " " << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()) << " -";
            THROW_EXCEPTION(err.str());
        }

        if (listen(fd, 128) < 0) {
            evutil_closesocket(fd);
            THROW_EXCEPTION("call listen");
        }
        LOG_TRACE << "listen ip:" << listen_addr_->ip() << " port:" << listen_addr_->port();
        listenfd_ = fd;
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
}
}
