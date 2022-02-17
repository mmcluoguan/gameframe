#include "shynet/thread/listenthread.h"
#include "shynet/net/listenreactormgr.h"
#include "shynet/utils/logger.h"
#include "shynet/utils/stuff.h"

namespace shynet {

extern pthread_barrier_t g_barrier;

namespace thread {
    ListenThread::ListenThread(size_t index)
        : Thread(ThreadType::LISTEN, index)
    {
    }

    int ListenThread::notify(int serverid) const
    {
        return pair_[0]->write(&serverid, sizeof(serverid));
    }

    static void pipeReadcb(struct bufferevent* bev, void* ptr)
    {
        try {
            ListenThread* rtk = reinterpret_cast<ListenThread*>(ptr);
            rtk->process(bev);
        } catch (const std::exception& err) {
            utils::stuff::print_exception(err);
        }
    }

    void ListenThread::process(bufferevent* bev)
    {
        events::EventBuffer pbuf(bev);
        int serverid = 0;
        do {

            size_t len = pbuf.read(&serverid, sizeof(serverid));
            if (len == 0) {
                break;
            } else if (len != sizeof(serverid)) {
                THROW_EXCEPTION("ListenThread没有足够的数据");
            } else {
                std::shared_ptr<net::ListenEvent> listenEv = utils::Singleton<net::ListenReactorMgr>::instance().find(serverid);
                if (listenEv != nullptr) {
                    listenEv->set_event(base_, listenEv->listenfd(), EV_READ | EV_PERSIST);
                    base_->addevent(listenEv, nullptr);
                }
            }
        } while (true);
    }

    int ListenThread::run()
    {
        LOG_TRACE << "ListenThread::run threadtype:" << (int)type();

        base_ = std::make_shared<events::EventBase>();
        base_->make_pair_buffer(pair_);
        pair_[0]->enabled(EV_WRITE);
        pair_[0]->disable(EV_READ);
        pair_[1]->enabled(EV_READ);
        pair_[1]->disable(EV_WRITE);
        pair_[1]->setcb(pipeReadcb, nullptr, nullptr, this);
        pthread_barrier_wait(&g_barrier);
        base_->loop(EVLOOP_NO_EXIT_ON_EMPTY);
        pair_[0].reset();
        pair_[1].reset();
        return 0;
    }
    int ListenThread::stop()
    {
        base_->loopexit();
        return 0;
    }
}
}
