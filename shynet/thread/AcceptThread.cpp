#include "shynet/thread/AcceptThread.h"
#include "shynet/net/AcceptHeartbeat.h"
#include "shynet/net/AcceptIoBuffer.h"
#include "shynet/net/ListenEvent.h"
#include "shynet/utils/Logger.h"
#include "shynet/utils/Stuff.h"

namespace shynet {

namespace thread {
    AcceptThread::AcceptThread(size_t index)
        : Thread(ThreadType::ACCEPT, index)
    {
    }

    AcceptThread::~AcceptThread()
    {
    }

    static void pipeReadcb(struct bufferevent* bev, void* ptr)
    {
        try {
            AcceptThread* rtk = reinterpret_cast<AcceptThread*>(ptr);
            rtk->process(bev);
        } catch (const std::exception& err) {
            utils::Stuff::print_exception(err);
        }
    }

    static std::mutex g_aptMutex;
    void AcceptThread::process(bufferevent* bev)
    {
        events::EventBuffer pbuf(bev);
        char buf[sizeof(uintptr_t)] = { 0 };
        do {
            size_t len = pbuf.read(&buf, sizeof(buf));
            if (len == 0) {
                break;
            } else if (len != sizeof(buf)) {
                LOG_DEBUG << "没有足够的数据";
            } else {
                uintptr_t* p = reinterpret_cast<uintptr_t*>(buf);
                net::ListenEvent* apnf = reinterpret_cast<net::ListenEvent*>(*p);

                struct sockaddr_storage cliaddr;
                memset(&cliaddr, 0, sizeof(cliaddr));
                socklen_t socklen = sizeof(cliaddr);

                int newfd;
                {
                    std::lock_guard<std::mutex> lock(g_aptMutex);
                    newfd = accept(apnf->listenfd(), (struct sockaddr*)&cliaddr, &socklen);
                    if (newfd < 0) {
                        return;
                    }
                    if (evutil_make_socket_nonblocking(apnf->listenfd()) < 0) {
                        evutil_closesocket(newfd);
                        THROW_EXCEPTION("call evutil_make_socket_nonblocking");
                    }
                }
                eventTot_++;
                std::shared_ptr<net::IPAddress> newfdAddr(new net::IPAddress(&cliaddr));
                LOG_TRACE << "accept newfd ip:" << newfdAddr->ip() << " port:" << newfdAddr->port();
                std::shared_ptr<net::AcceptIoBuffer> iobuf;
                if (apnf->enable_ssl()) {
                    iobuf = std::shared_ptr<net::AcceptIoBuffer>(
                        new net::AcceptIoBuffer(base_, newfd, true, apnf->ctx()));
                } else {
                    iobuf = std::shared_ptr<net::AcceptIoBuffer>(new net::AcceptIoBuffer(base_, newfd, false));
                }
                std::shared_ptr<net::AcceptNewFd> apnewfd = apnf->accept_newfd(newfdAddr, iobuf).lock();
                iobuf->set_newfd(apnewfd);
                if (apnewfd->enableHeart()) {
                    std::shared_ptr<net::AcceptHeartbeat> ht(
                        new net::AcceptHeartbeat(apnewfd, { apnewfd->heart_second(), 0L }));
                    utils::Singleton<net::TimerReactorMgr>::instance().add(ht);
                    apnewfd->set_heart(ht);
                }
            }
        } while (true);
    }

    int AcceptThread::run()
    {
        try {
            LOG_TRACE << "AcceptThread::run threadtype:" << (int)type();

            base_ = std::shared_ptr<events::EventBase>(new events::EventBase());
            base_->make_pair_buffer(pair_);
            pair_[0]->enabled(EV_WRITE);
            pair_[0]->disable(EV_READ);
            pair_[1]->enabled(EV_READ);
            pair_[1]->disable(EV_WRITE);
            pair_[1]->setcb(pipeReadcb, nullptr, nullptr, this);
            base_->loop(EVLOOP_NO_EXIT_ON_EMPTY);
            pair_[0].reset();
            pair_[1].reset();
        } catch (const std::exception& err) {
            utils::Stuff::print_exception(err);
        }
        return 0;
    }
    int AcceptThread::stop()
    {
        base_->loopexit();
        return 0;
    }

    int AcceptThread::notify(const void* data, size_t len) const
    {
        return pair_[0]->write(data, len);
    }
}
}
