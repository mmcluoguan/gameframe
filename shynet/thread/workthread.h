#ifndef SHYNET_THREAD_WORKTHREAD_H
#define SHYNET_THREAD_WORKTHREAD_H

#include "shynet/thread/thread.h"
#include <mutex>
#include <queue>

namespace shynet {
namespace thread {
    /**
     * @brief work线程
    */
    class WorkThread : public Thread {
    public:
        /**
         * @brief 构造
         * @param index 线程在线程池中的索引
        */
        explicit WorkThread(size_t index);
        ~WorkThread() = default;

        /**
         * @brief 线程运行回调
         * @return 0成功 -1失败
        */
        int run() override;
        /**
         * @brief 安全终止线程
         * @return 0成功 -1失败
        */
        int stop() override;

        /**
         * @brief 添加工作任务
         * @param tk 工作任务
        */
        void addTask(std::function<void()>&& tk);

    private:
        /**
         * @brief 互斥体
        */
        std::mutex tasks_mutex_;
        /**
         * @brief 工作任务队列
        */
        std::queue<std::function<void()>> tasks_;
        /**
         * @brief 线程结束标识
        */
        bool stop_ = false;
    };
}
}

#endif
