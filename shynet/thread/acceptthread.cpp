#include "shynet/thread/acceptthread.h"
#include "shynet/net/acceptheartbeat.h"
#include "shynet/net/acceptiobuffer.h"
#include "shynet/net/listenreactormgr.h"
#include "shynet/pool/threadpool.h"
#include "shynet/thread/udpthread.h"
#include "shynet/utils/logger.h"
#include "shynet/utils/stuff.h"
#include <atomic>

namespace shynet {

namespace thread {
    AcceptThread::AcceptThread(size_t index)
        : Thread(ThreadType::ACCEPT, index)
    {
    }

    static void pipeReadcb(struct bufferevent* bev, void* ptr)
    {
        try {
            AcceptThread* rtk = reinterpret_cast<AcceptThread*>(ptr);
            rtk->process(bev);
        } catch (const std::exception& err) {
            utils::stuff::print_exception(err);
        }
    }

    void AcceptThread::process(bufferevent* bev)
    {
        events::EventBuffer pbuf(bev);
        int serverid = 0;
        do {

            size_t len = pbuf.read(&serverid, sizeof(serverid));
            if (len == 0) {
                break;
            } else if (len != sizeof(serverid)) {
                THROW_EXCEPTION("AcceptThread没有足够的数据");
            } else {
                auto apnf = utils::Singleton<net::ListenReactorMgr>::instance().find(serverid);
                if (apnf) {
                    if (apnf->type() == SOCK_STREAM) {
                        tcp_accept(apnf);
                    } else if (apnf->type() == SOCK_DGRAM) {
                        auto udpth = utils::Singleton<pool::ThreadPool>::instance().udpTh().lock();
                        if (udpth) {
                            int ident = static_cast<int>(protocol::FilterProces::Identity::ACCEPTOR);
                            udpth->notify(ident, serverid);
                        } else {
                            THROW_EXCEPTION("udpthread 未创建");
                        }
                    }
                }
            }
        } while (true);
    }

    void AcceptThread::tcp_accept(std::shared_ptr<net::ListenEvent> apnf)
    {
        struct sockaddr_storage cliaddr;
        memset(&cliaddr, 0, sizeof(cliaddr));
        socklen_t socklen = sizeof(cliaddr);

        int newfd = accept(apnf->listenfd(), (struct sockaddr*)&cliaddr, &socklen);
        if (newfd < 0) {
            return;
        }
        if (evutil_make_socket_nonblocking(apnf->listenfd()) < 0) {
            evutil_closesocket(newfd);
            THROW_EXCEPTION("call evutil_make_socket_nonblocking");
        }
        eventTot_++;
        std::shared_ptr<net::IPAddress> newfdAddr = std::make_shared<net::IPAddress>(&cliaddr);
        LOG_TRACE << "accept newfd ip:" << newfdAddr->ip() << " port:" << newfdAddr->port();
        std::shared_ptr<net::AcceptIoBuffer> iobuf;
        if (apnf->enable_ssl()) {
            iobuf = std::make_shared<net::AcceptIoBuffer>(
                base_, newfd, true, apnf->ctx());
        } else {
            iobuf = std::make_shared<net::AcceptIoBuffer>(
                base_, newfd, false);
        }
        std::shared_ptr<net::AcceptNewFd> apnewfd = apnf->accept_newfd(newfdAddr, iobuf).lock();
        iobuf->set_newfd(apnewfd);
        if (apnewfd->enable_check()) {
            //设置检测与服务器连接状态计时器
            std::shared_ptr<net::AcceptHeartbeat> ht = std::make_shared<net::AcceptHeartbeat>(
                apnewfd, timeval { apnewfd->check_second(), 0L });
            int id = utils::Singleton<net::TimerReactorMgr>::instance().add(ht);
            apnewfd->set_check_timeid(id);
        }
    }

    int AcceptThread::run()
    {
        LOG_TRACE << "AcceptThread::run threadtype:" << (int)type();

        base_ = std::make_shared<events::EventBase>();
        base_->make_pair_buffer(pair_);
        pair_[0]->enabled(EV_WRITE);
        pair_[0]->disable(EV_READ);
        pair_[1]->enabled(EV_READ);
        pair_[1]->disable(EV_WRITE);
        pair_[1]->setcb(pipeReadcb, nullptr, nullptr, this);
        base_->loop(EVLOOP_NO_EXIT_ON_EMPTY);
        pair_[0].reset();
        pair_[1].reset();
        return 0;
    }
    int AcceptThread::stop()
    {
        base_->loopexit();
        return 0;
    }

    int AcceptThread::notify(int serverid) const
    {
        return pair_[0]->write(&serverid, sizeof(serverid));
    }
}
}
