#ifndef SHYNET_THREAD_CONNECTTHREAD_H
#define SHYNET_THREAD_CONNECTTHREAD_H

#include "shynet/events/eventbuffer.h"
#include "shynet/net/connectevent.h"
#include "shynet/thread/thread.h"

namespace shynet {
namespace thread {
    /**
     * @brief 连接服务器线程
    */
    class ConnectThread : public Thread {
    public:
        /**
         * @brief 构造
         * @param index 线程在线程池中的索引
        */
        explicit ConnectThread(size_t index);
        ~ConnectThread() = default;

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
         * @brief 通知连接服务器线程接收新的连接
         * @param data 客户端id的地址
         * @param len 客户端id地址大小
        */
        int notify(const void* data, size_t len) const;

        /**
         * @brief 处理通知来的信息
         * @param bev 通知来的信息
        */
        void process(struct bufferevent* bev);

    private:
        /**
         * @brief 开始tcp连接
         * @param connect 接收服务器数据处理器
        */
        void tcp_connect(std::shared_ptr<net::ConnectEvent> connect);
        /**
         * @brief 开始udp连接
         * @param connect 接收服务器数据处理器
        */
        void udp_connect(std::shared_ptr<net::ConnectEvent> connect);

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
