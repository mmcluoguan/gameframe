#include "shynet/net/timerreactormgr.h"
#include "shynet/pool/threadpool.h"
#include "shynet/thread/timerthread.h"

namespace shynet {
namespace net {

    TimerReactorMgr::TimerReactorMgr() { }

    TimerReactorMgr::~TimerReactorMgr() { }

    void TimerReactorMgr::notify(const void* data, size_t len)
    {
        std::shared_ptr<thread::TimerThread> tth = utils::Singleton<pool::ThreadPool>::get_instance().timerTh().lock();
        if (tth != nullptr) {
            tth->notify(data, len);
        } else
            LOG_WARN << "没有可用的 TimerThread";
    }

    int TimerReactorMgr::add(std::shared_ptr<TimerEvent> v)
    {
        static int timerid = 0;
        {
            std::lock_guard<std::mutex> lock(times_mutex_);
            timerid++;
            times_.insert({ timerid, v });
            v->set_timerid(timerid);
        }
        notify(&timerid, sizeof(timerid));
        return timerid;
    }

    bool TimerReactorMgr::remove(int k)
    {
        std::lock_guard<std::mutex> lock(times_mutex_);
        return times_.erase(k);
    }

    std::shared_ptr<TimerEvent> TimerReactorMgr::find(int k)
    {
        std::lock_guard<std::mutex> lock(times_mutex_);
        return times_[k];
    }
} // namespace net
} // namespace shynet
