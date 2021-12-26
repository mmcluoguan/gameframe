#ifndef SHYNET_THREAD_INOTIFYTHREAD_H
#define SHYNET_THREAD_INOTIFYTHREAD_H

#include "shynet/events/eventbuffer.h"
#include "shynet/task/notifytask.h"
#include "shynet/thread/thread.h"
#include <mutex>

namespace shynet {
namespace thread {
    /**
     * @brief 目录监控线程
    */
    class InotifyThread : public Thread {
    public:
        /**
         * @brief 构造
         * @param index 线程在线程池中的索引
        */
        explicit InotifyThread(size_t index);
        ~InotifyThread();

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
         * @brief 监控的目录发生事件
        */
        void io_readcb();

        /**
         * @brief 添加目录监控任务
         * @param task 目录监控任务
        */
        void add(std::shared_ptr<task::NotifyTask> task);

        /**
         * @brief 移除目录监控任务
         * @param task 目录监控任务
        */
        void remove(std::shared_ptr<task::NotifyTask> task);

    private:
        /**
         * @brief 互斥体
        */
        std::mutex task_mutex_;
        /**
         * @brief 目录文件描述符对应的目录监控任务的hash表
        */
        std::unordered_map<int, std::shared_ptr<task::NotifyTask>> task_map_;
        /**
         * @brief 监控文件描述符
        */
        int notifyfd_;
        /**
         * @brief 反应器
        */
        std::shared_ptr<events::EventBase> base_ = nullptr;
        /**
         * @brief 管理io缓冲
        */
        std::shared_ptr<events::EventBuffer> iobuf_ = nullptr;
    };
}
}

#endif
