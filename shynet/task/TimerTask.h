#ifndef SHYNET_TASK_TIMERTASK_H
#define SHYNET_TASK_TIMERTASK_H

#include "shynet/task/Task.h"

namespace shynet {
namespace task {
    class TimerTask : public task::Task {
    public:
        explicit TimerTask(int timerid);
        ~TimerTask();

        int run(thread::Thread* tif) override;

    private:
        int timerid_ = 0;
    };
}
}

#endif
