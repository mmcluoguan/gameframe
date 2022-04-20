#ifndef SHYNET_PROTOCOL_UDPSOCKET_H
#define SHYNET_PROTOCOL_UDPSOCKET_H

#include "shynet/events/eventbuffer.h"
#include "shynet/net/ipaddress.h"
#include "shynet/protocol/filterproces.h"
#include "shynet/protocol/ikcp.h"
#include <mutex>

namespace shynet {
namespace net {
    class ConnectEvent;
    class AcceptNewFd;
}
namespace protocol {
#define MAXIMUM_MTU_SIZE 1400

    enum class UdpMessageDefine : char {
        //尝试连接
        ID_ATTEMPT_CONNECT,
        //尝试连接回应
        ID_ATTEMPT_CONNECT_ACK,
        //请求连接
        ID_CONNECT,
        //对端关闭
        ID_CLOSE,
    };

    /**
     * @brief udp协议处理器
    */
    class UdpSocket : public Nocopy {
    public:
        explicit UdpSocket(FilterProces::Identity ident);
        ~UdpSocket();

        enum state { CONNECTING,
            CONNECTSUCCESS,
            CONNECTFAIL };
        /**
        * @brief 下次请求连接时间
        */
        int64_t next_request_time = 0;
        /**
        * @brief 累计连接超时次数
        */
        const int32_t send_connection_attempt_count = 10;
        /**
        * @brief 下次连接重试间隔ms
        */
        const int32_t next_requset_interval = 500;
        /**
         * @brief 当前连接状态
        */
        state st = CONNECTING;
        /**
        * @brief 当前连接尝试次数
        */
        int32_t curr_request_num = 0;
        /**
         * @brief 对端地址
        */
        net::IPAddress addr;
        /**
         * @brief 文件描述符
        */
        int fd = -1;

        /**
         * @brief 输入缓冲区
        */
        std::shared_ptr<events::EventBuffer> input_buffer;
        /**
         * @brief 输出缓冲区
        */
        std::shared_ptr<events::EventBuffer> output_buffer;

        /**
         * @brief 接收客户端数据处理器
        */
        std::weak_ptr<net::AcceptNewFd> client;
        /**
         * @brief 接收服务器数据处理器
        */
        std::weak_ptr<net::ConnectEvent> cnev;

        /**
         * @brief 初始化输入输出缓冲区
         * @param base 反应堆
        */
        void init_pair_buffer(std::shared_ptr<events::EventBase> base);

        /**
         * @brief kcp发送数据
         * @param data 
         * @param size 
         * @return 发送到kcp缓冲区的大小
        */
        int send(const char* data, size_t size);
        /**
         * @brief 原生udp发送数据
         * @param data 
         * @param size 
         * @return 发送的字节数
        */
        ssize_t sendto(const char* data, size_t size);

        /**
         * @brief kcp接收数据
         * @param data 
         * @param size 
         * @return 返回接收的字节数
        */
        int recv(char* data, size_t size);
        /**
         * @brief 原生udp接收数据
         * @param data 
         * @param size 
         * @return 返回接收的字节数
        */
        ssize_t recvfrom(char* data, size_t size);

        /**
         * @brief 设置kcp标识
         * @param guid 唯一标识
        */
        void set_guid(uint32_t guid);
        /**
         * @brief 获取kcp标识
         * @return kcp标识
        */
        uint32_t guid() const { return guid_; };
        /**
         * @brief 获取kcp指针
         * @return kcp指针
        */
        ikcpcb* kcp() const { return kcp_; };
        /**
         * @brief 获取身份标识
         * @return 身份标识
        */
        FilterProces::Identity ident() const { return ident_; };

    private:
        /**
         * @brief kcp
        */
        ikcpcb* kcp_ = nullptr;
        /**
         * @brief 唯一id
        */
        uint32_t guid_ = -1;
        /**
         * @brief 身份标识
        */
        FilterProces::Identity ident_;
        /**
         * @brief 接收缓存累计大小
        */
        size_t recv_buffer_length_ = 0;
        /**
         * @brief 发送窗口大小
        */
        const int32_t sndwnd_size_ = 128;
        /**
         * @brief 接收窗口大小
        */
        const int32_t rcvwnd_size_ = 128;
    };
}
}

#endif
