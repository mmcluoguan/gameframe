#ifndef SHYNET_TASK_TIMERTASK_H
#define SHYNET_TASK_TIMERTASK_H

namespace shynet {
namespace task {
    class TimerTask {
    public:
        explicit TimerTask(int timerid);
        ~TimerTask();

        void operator()();

    private:
        int timerid_ = 0;
    };
}
}

#endif
