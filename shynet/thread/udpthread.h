#ifndef SHYNET_THREAD_UDPTHREAD_H
#define SHYNET_THREAD_UDPTHREAD_H

#include "shynet/net/connectevent.h"
#include "shynet/net/listenevent.h"
#include "shynet/protocol/udpsocket.h"
#include "shynet/thread/thread.h"
#include <chrono>
#include <list>
#include <mutex>

namespace shynet {
namespace thread {
    /**
     * @brief udp逻辑线程
    */
    class UdpThread : public Thread {
    public:
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
         * @brief 通知udp线程有数据可以接收
         * @param ident 
         * @param targetid
        */
        int notify(int ident, int targetid) const;

        /**
         * @brief 处理通知来的信息
         * @param bev 通知来的信息
        */
        void process(struct bufferevent* bev);

        /**
         * @brief 添加sock到等待连接列表
         * @param connectid 连接器id
        */
        void add_waitconnect(int connectid);

        /**
         * @brief 添加已连接完成sock到已连接成功列表
         * @param ip 地址
         * @param sock udp协议处理器
        */
        void add_accept_udp(net::IPAddress& ip, std::weak_ptr<protocol::UdpSocket> sock);
        /**
         * @brief 从已连接成功列表中移除udp协议处理器
         * @param ip 地址
        */
        void remove_accept_udp(net::IPAddress& ip);
        /**
         * @brief 从已连接成功列表中查找udp协议处理器
         * @param ip 地址
         * @return 返回nullptr表示未找到
        */
        std::weak_ptr<protocol::UdpSocket> find_accept_udp(net::IPAddress& ip);

        /**
         * @brief 添加客户端已连接完成sock到已连接成功列表
         * @param guid 唯一id
         * @param sock udp协议处理器
        */
        void add_connect_udp(uint32_t guid, std::weak_ptr<protocol::UdpSocket> sock);
        /**
         * @brief 从已连接成功列表中移除客户端已连接完成sock
         * @param guid 唯一id
        */
        void remove_connect_udp(uint32_t guid);

        /**
         * @brief 处理等待连接的队列
         * @param elapsed_num 
        */
        void handler_waitconnenct(int64_t elapsed_num);
        /**
         * @brief 处理已连接的udp连接
         * @param elapsed_num 
        */
        void handler_connected(int64_t elapsed_num);
        /**
         * @brief 处理连接udp服务器的client
         * @param elapsed_num 
        */
        void handler_accpeted(int64_t elapsed_num);

        /**
         * @brief 获取线程开始时间
         * @return 线程开始时间
        */
        std::chrono::time_point<std::chrono::steady_clock> start_time() const { return start_time_; };

        /**
         * @brief 连接服务器
         * @param connect 接收服务器数据处理器 
        */
        void connect_server(std::shared_ptr<net::ConnectEvent> connect);

    private:
        /**
         * @brief 接收端可靠udp层处理消息
        */
        void handler_accept_reliable(std::shared_ptr<protocol::UdpSocket> sock, char* buffer, ssize_t size);
        /**
         * @brief 接收端不可靠udp层处理消息
        */
        void handler_accept_no_reliable(std::shared_ptr<net::ListenEvent> apnf, sockaddr_storage& cliaddr, char* buffer, ssize_t size);

        /**
         * @brief 连接端可靠udp层处理消息
        */
        void handler_connect_reliable(std::shared_ptr<protocol::UdpSocket> sock, char* buffer, ssize_t size);
        /**
         * @brief 连接端不可靠udp层处理消息
        */
        void handler_connect_no_reliable(std::shared_ptr<net::ConnectEvent> cnt, char* buffer, ssize_t size);

        std::mutex waitconnect_list_mtx_;
        /**
         * @brief 等待连接列表
        */
        std::list<int> waitconnect_list_;

        std::mutex accept_udp_mtx_;
        /**
         * @brief 服务器已连接hash表
        */
        std::unordered_map<net::IPAddress, std::weak_ptr<protocol::UdpSocket>, net::IPAddressHash> accept_udp_layer_;

        std::mutex connect_udp_mtx_;
        /**
         * @brief 客户端已连接hash表
        */
        std::unordered_map<uint32_t, std::weak_ptr<protocol::UdpSocket>> connect_udp_layer_;

        /**
         * @brief 接收通知的管道
        */
        std::shared_ptr<events::EventBuffer> pair_[2] = { 0 };
        /**
         * @brief 反应器
        */
        std::shared_ptr<events::EventBase> base_ = nullptr;
        /**
         * @brief 更新毫秒时间
        */
        int64_t updata_elapsed_ = 20;
        /**
         * @brief 线程开始时间
        */
        std::chrono::time_point<std::chrono::steady_clock> start_time_;
        /**
         * @brief 更新计时事件
        */
        struct event* updata_timer_;
    };
}
}

#endif
