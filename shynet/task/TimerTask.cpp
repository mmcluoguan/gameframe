#include "shynet/task/timertask.h"
#include "shynet/net/timerreactormgr.h"

namespace shynet {
namespace task {

    TimerTask::TimerTask(int timerid)
    {
        timerid_ = timerid;
    }
    TimerTask::~TimerTask()
    {
    }

    int TimerTask::run(thread::Thread* tif)
    {
        std::shared_ptr<net::TimerEvent> tv = utils::Singleton<net::TimerReactorMgr>::instance().find(timerid_);
        if (tv != nullptr) {
            tv->timeout();
        }
        return 0;
    }
}
}
