#include "shynet/task/timertask.h"
#include "shynet/net/timerreactormgr.h"

namespace shynet {
namespace task {

    TimerTask::TimerTask(int timerid)
    {
        timerid_ = timerid;
    }

    void TimerTask::operator()()
    {
        auto& timermgr = utils::Singleton<net::TimerReactorMgr>::instance();
        std::shared_ptr<net::TimerEvent> tv = timermgr.find(timerid_);
        if (tv != nullptr) {
            tv->timeout();
            if (!(tv->what() & EV_PERSIST)) {
                timermgr.remove(timerid_);
            }
        }
    }
}
}
