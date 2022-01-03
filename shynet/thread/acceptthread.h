#ifndef SHYNET_THREAD_ACCEPTTHREAD_H
#define SHYNET_THREAD_ACCEPTTHREAD_H

#include "shynet/events/eventbuffer.h"
#include "shynet/thread/thread.h"

namespace shynet {
namespace thread {
    /**
     * @brief 服务器接收客户端数据线程
    */
    class AcceptThread : public Thread {
    public:
        /**
         * @brief 构造
         * @param index 线程在线程池中的索引 
        */
        explicit AcceptThread(size_t index);
        ~AcceptThread() = default;

        /**
         * @brief 获取累计接收客户端连接数量
         * @return 累计接收客户端连接数量
        */
        int event_tot() const { return eventTot_; }

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
         * @brief 通知服务器接收客户端数据线程接收新的连接
         * @param data 数据为 ListenEvent*
         * @param len 数据长度
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
        /**
         * @brief 累计接收客户端连接数量
        */
        int eventTot_ = 0;
    };
}
}

#endif
