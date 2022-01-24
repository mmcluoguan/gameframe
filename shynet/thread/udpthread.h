#ifndef SHYNET_THREAD_UDPTHREAD_H
#define SHYNET_THREAD_UDPTHREAD_H

#include "shynet/net/ipaddress.h"
#include "shynet/protocol/ikcp.h"
#include "shynet/thread/thread.h"
#include <mutex>
#include <unordered_map>

namespace shynet {
namespace thread {
    /**
     * @brief udp逻辑线程
    */
    class UdpThread : public Thread {
    public:
        /**
         * @brief UdpSocket
        */
        class UdpSocket {
        public:
            UdpSocket(int32_t key, net::IPAddress* addr);
            ~UdpSocket();

            enum state { CONNECTING,
                CONNECTED };

            int send(const char* data, size_t size);
            int sendto(const char* data, size_t size);

            /**
             * @brief 下次请求连接时间
            */
            int64_t next_request_time = 0;
            /**
             * @brief 累计连接超时次数
            */
            const int32_t send_connection_attempt_count = 12;
            /**
             * @brief 下次连接重试间隔ms
            */
            const int32_t next_requset_interval = 100;
            /**
             * @brief 当前连接状态
            */
            state st = CONNECTING;
            /**
             * @brief 当前连接尝试次数
            */
            int32_t curr_request_num = 0;

        private:
            int32_t key_;
            net::IPAddress addr_;
            int fd_ = -1;
            ikcpcb* kcp_ = nullptr;
        };

        /**
         * @brief 构造
         * @param index 线程在线程池中的索引
        */
        explicit UdpThread(size_t index);
        ~UdpThread() = default;

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
         * @brief 添加新udp连接请求到等待连接的hash映射表
         * @param key 连接的服务器地址管理器分配的id
        */
        void add(int32_t key, net::IPAddress* addr);

    private:
        /**
         * @brief 线程结束标识
        */
        bool stop_ = false;

        std::mutex waitconnect_map_mtx_;
        /**
         * @brief 等待连接的hash映射表,
        key为连接的服务器地址管理器分配的id,value位置udpsocket
        */
        std::unordered_map<int32_t, std::shared_ptr<UdpSocket>> waitconnect_map_;
    };
}
}

#endif
