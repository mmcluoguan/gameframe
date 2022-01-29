#ifndef SHYNET_THREAD_LISTENTHREAD_H
#define SHYNET_THREAD_LISTENTHREAD_H

#include "shynet/events/eventbuffer.h"
#include "shynet/thread/thread.h"

namespace shynet {
namespace thread {
    /**
     * @brief 侦听的服务器地址线程
    */
    class ListenThread : public Thread {
    public:
        /**
         * @brief 构造
         * @param index 线程在线程池中的索引
        */
        explicit ListenThread(size_t index);
        ~ListenThread() = default;

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
         * @brief 通知侦听的服务器地址线程有新的服务器地址需要侦听
         * @param serverid
        */
        int notify(int serverid) const;

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
