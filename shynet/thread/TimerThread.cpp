#include "shynet/thread/TimerThread.h"
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

    TimerThread::~TimerThread()
    {
    }

    int TimerThread::notify(const void* data, size_t len) const
    {
        return pair_[0]->write(data, len);
    }

    static void pipeReadcb(struct bufferevent* bev, void* ptr)
    {
        try {
            TimerThread* rtk = reinterpret_cast<TimerThread*>(ptr);
            rtk->process(bev);
        } catch (const std::exception& err) {
            utils::Stuff::print_exception(err);
        }
    }

    void TimerThread::process(bufferevent* bev)
    {
        events::EventBuffer pbuf(bev);
        char buf[sizeof(int)] = { 0 };
        do {
            size_t len = pbuf.read(&buf, sizeof(buf));
            if (len == 0) {
                break;
            } else if (len != sizeof(buf)) {
                LOG_WARN << "没有足够的数据";
            } else {
                int timerid = 0;
                size_t index = 0;
                memcpy(&timerid, buf + index, sizeof(timerid));
                index += sizeof(timerid);

                std::shared_ptr<net::TimerEvent> timerEv = utils::Singleton<net::TimerReactorMgr>::instance().find(timerid);
                if (timerEv != nullptr) {
                    timerEv->event(base_, -1, timerEv->what());
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

            base_ = std::shared_ptr<events::EventBase>(new events::EventBase());
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
            utils::Stuff::print_exception(err);
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
