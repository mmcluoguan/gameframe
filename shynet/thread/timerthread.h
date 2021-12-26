#ifndef SHYNET_THREAD_TIMERTHREAD_H
#define SHYNET_THREAD_TIMERTHREAD_H

#include "shynet/events/eventbuffer.h"
#include "shynet/thread/thread.h"

namespace shynet {
namespace thread {
    /**
     * @brief 计时器线程
    */
    class TimerThread : public Thread {
    public:
        /**
         * @brief 构造
         * @param index 线程在线程池中的索引
        */
        explicit TimerThread(size_t index);
        ~TimerThread() = default;

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
         * @brief 通知计时器线程有新的计时器加入
         * @param data 计时器id的地址
         * @param len 计时器id的地址大小
        */
        int notify(const void* data, size_t len) const;

        /**
         * @brief 处理通知来的信息
         * @param bev 通知来的信息
        */
        void process(struct bufferevent* bev);

    private:
        /**
         * @brief 接收通知的管道
        */
        std::shared_ptr<events::EventBuffer> pair_[2] = { 0 };
        /**
         * @brief 反应器
        */
        std::shared_ptr<events::EventBase> base_ = nullptr;
    };
}
}

#endif
