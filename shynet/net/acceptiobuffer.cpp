#include "shynet/net/acceptiobuffer.h"
#include "shynet/net/acceptreactormgr.h"
#include "shynet/net/timerreactormgr.h"
#include "shynet/pool/threadpool.h"
#include "shynet/task/acceptreadiotask.h"
#include "shynet/utils/stuff.h"

namespace shynet {
namespace net {
    static void ioreadcb(struct bufferevent* bev, void* ptr)
    {
        AcceptIoBuffer* p = reinterpret_cast<AcceptIoBuffer*>(ptr);
        p->io_readcb();
    }

    static void iowritecb(struct bufferevent* bev, void* ptr)
    {
        AcceptIoBuffer* p = reinterpret_cast<AcceptIoBuffer*>(ptr);
        p->io_writecb();
    }

    static void ioeventcb(struct bufferevent* bev, short events, void* ptr)
    {
        AcceptIoBuffer* p = reinterpret_cast<AcceptIoBuffer*>(ptr);
        p->io_eventcb(events);
    }

    AcceptIoBuffer::AcceptIoBuffer(std::shared_ptr<events::EventBase> base,
        evutil_socket_t fd,
        bool enable_ssl,
        SSL_CTX* ctx)
        : events::EventBuffer(base)
    {
        if (enable_ssl) {
            iobuf_ = std::make_shared<events::EventBufferSsl>(base, fd,
                BUFFEREVENT_SSL_ACCEPTING,
                BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE, ctx);
        } else {
            iobuf_ = std::make_shared<events::EventBuffer>(base, fd,
                BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);
        }
        set_buffer(iobuf_->buffer());
        set_fd(fd);

        iobuf_->setcb(ioreadcb, iowritecb, ioeventcb, this);
        iobuf_->enabled(EV_READ | EV_WRITE | EV_PERSIST);
    }

    std::weak_ptr<AcceptNewFd> AcceptIoBuffer::newfd() const
    {
        return newfd_;
    }

    void AcceptIoBuffer::set_newfd(std::weak_ptr<AcceptNewFd> newfd)
    {
        newfd_ = newfd;
    }

    void AcceptIoBuffer::io_readcb()
    {
        try {
            std::shared_ptr<AcceptNewFd> aptnewfd = newfd_.lock();
            if (aptnewfd != nullptr) {
                if (aptnewfd->enable_check()) {
                    //延迟检测与客户端连接状态的计时处理器时间
                    auto heart = utils::Singleton<TimerReactorMgr>::instance().find(aptnewfd->check_timeid());
                    if (heart) {
                        heart->set_val({ aptnewfd->check_second(), 0L });
                    }
                }
                net::InputResult ret = aptnewfd->input([aptnewfd](std::unique_ptr<char[]> data, size_t len) {
                    std::shared_ptr<task::AcceptReadIoTask> io
                        = std::make_shared<task::AcceptReadIoTask>(aptnewfd, std::move(data), len);
                    utils::Singleton<pool::ThreadPool>::instance().appendwork(aptnewfd->iobuf()->fd(), io);
                });
                if (ret == net::InputResult::INITIATIVE_CLOSE) {
                    aptnewfd->close(net::CloseType::SERVER_CLOSE);
                } else if (ret == net::InputResult::PASSIVE_CLOSE) {
                    aptnewfd->close(net::CloseType::CLIENT_CLOSE);
                }
            }
        } catch (const std::exception& err) {
            utils::stuff::print_exception(err);
        }
    }

    void AcceptIoBuffer::io_writecb()
    {
        try {
            std::shared_ptr<AcceptNewFd> aptnewfd = newfd_.lock();
            if (aptnewfd != nullptr) {
                int ret = aptnewfd->output();
                if (ret == -1) {
                    aptnewfd->close(net::CloseType::SERVER_CLOSE);
                }
            }
        } catch (const std::exception& err) {
            utils::stuff::print_exception(err);
        }
    }

    void AcceptIoBuffer::io_eventcb(short events)
    {
        try {
            std::shared_ptr<AcceptNewFd> aptnewfd = newfd_.lock();
            if (aptnewfd != nullptr) {
                if (events & BEV_EVENT_EOF) {
                    aptnewfd->close(net::CloseType::CLIENT_CLOSE);
                } else if (events & (BEV_EVENT_ERROR | BEV_EVENT_READING | BEV_EVENT_WRITING | BEV_EVENT_TIMEOUT)) {
                    LOG_WARN << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR());
                    aptnewfd->close(net::CloseType::SERVER_CLOSE);
                }
            }
        } catch (const std::exception& err) {
            utils::stuff::print_exception(err);
        }
    }

}
}
