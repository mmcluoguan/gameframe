#ifndef SHYNET_TASK_TIMERTASK_H
#define SHYNET_TASK_TIMERTASK_H

namespace shynet {
namespace task {
    /**
     * @brief 计时器超时任务处理器
    */
    class TimerTask {
    public:
        /**
         * @brief 构造
         * @param timerid 计时器id 
        */
        explicit TimerTask(int timerid);
        ~TimerTask() = default;

        /**
         * @brief 任务处理
        */
        void operator()();

    private:
        /**
         * @brief 计时器id 
        */
        int timerid_ = 0;
    };
}
}

#endif
