#include "shynet/thread/timerthread.h"
#include "shynet/net/timerreactormgr.h"
#include "shynet/utils/stuff.h"
#include <cstring>

namespace shynet {

extern pthread_barrier_t g_barrier;

namespace thread {
    TimerThread::TimerThread(size_t index)
        : Thread(ThreadType::TIMER, index)
    {
    }

    int TimerThread::notify(int timerid) const
    {
        return pair_[0]->write(&timerid, sizeof(timerid));
    }

    static void pipeReadcb(struct bufferevent* bev, void* ptr)
    {
        try {
            TimerThread* rtk = reinterpret_cast<TimerThread*>(ptr);
            rtk->process(bev);
        } catch (const std::exception& err) {
            utils::stuff::print_exception(err);
        }
    }

    void TimerThread::process(bufferevent* bev)
    {
        events::EventBuffer pbuf(bev);
        int timerid = 0;
        do {
            size_t len = pbuf.read(&timerid, sizeof(timerid));
            if (len == 0) {
                break;
            } else if (len != sizeof(timerid)) {
                THROW_EXCEPTION("TimerThread没有足够的数据");
            } else {
                std::shared_ptr<net::TimerEvent> timerEv = utils::Singleton<net::TimerReactorMgr>::instance().find(timerid);
                if (timerEv != nullptr) {
                    timerEv->set_event(base_, -1, timerEv->what());
                    base_->addevent(timerEv, &timerEv->val());
                } else {
                    LOG_TRACE << "set_timerid:" << timerid << " not exist";
                }
            }
        } while (true);
    }

    int TimerThread::run()
    {
        try {
            LOG_TRACE << "TimerThread::run threadtype:" << (int)type();

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
        } catch (const std::exception& err) {
            utils::stuff::print_exception(err);
        }
        return 0;
    }
    int TimerThread::stop()
    {
        base_->loopexit();
        return 0;
    }
}
}
