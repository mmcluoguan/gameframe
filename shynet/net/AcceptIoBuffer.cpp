#include "shynet/net/AcceptIoBuffer.h"
#include "shynet/net/AcceptReactorMgr.h"
#include "shynet/pool/ThreadPool.h"
#include "shynet/task/AcceptReadIoTask.h"

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

        iobuf_->setcb(ioreadcb, iowritecb, ioeventcb, this);
        iobuf_->enabled(EV_READ | EV_WRITE | EV_PERSIST);
    }
    AcceptIoBuffer::~AcceptIoBuffer()
    {
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
        std::shared_ptr<AcceptNewFd> aptnewfd = newfd_.lock();
        if (aptnewfd != nullptr) {
            std::shared_ptr<task::AcceptReadIoTask> io = std::make_shared<task::AcceptReadIoTask>(aptnewfd);
            utils::Singleton<pool::ThreadPool>::instance().appendWork(io, fd());
        }
    }

    void AcceptIoBuffer::io_writecb()
    {
        std::shared_ptr<AcceptNewFd> aptnewfd = newfd_.lock();
        if (aptnewfd != nullptr) {
            int ret = aptnewfd->output();
            if (ret == -1) {
                aptnewfd->close(true);
            }
        }
    }

    void AcceptIoBuffer::io_eventcb(short events)
    {
        std::shared_ptr<AcceptNewFd> aptnewfd = newfd_.lock();
        if (aptnewfd != nullptr) {
            if (events & BEV_EVENT_EOF) {
                aptnewfd->close(false);
            } else if (events & (BEV_EVENT_ERROR | BEV_EVENT_READING | BEV_EVENT_WRITING)) {
                LOG_WARN << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR());
                aptnewfd->close(true);
            }
        }
    }

}
}
