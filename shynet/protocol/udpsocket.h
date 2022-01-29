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
        /**
         * @brief 对端地址
        */
        net::IPAddress addr;
        /**
         * @brief 文件描述符
        */
        int fd = -1;

        std::shared_ptr<events::EventBuffer> input_buffer;
        std::shared_ptr<events::EventBuffer> output_buffer;

        std::weak_ptr<net::AcceptNewFd> client;
        std::weak_ptr<net::ConnectEvent> cnev;

        void init_pair_buffer(std::shared_ptr<events::EventBase> base);

        int send(const char* data, size_t size);
        ssize_t sendto(const char* data, size_t size);

        int recv(char* data, size_t size);
        ssize_t recvfrom(char* data, size_t size);

        void set_guid(uint32_t guid);
        uint32_t guid() const { return guid_; };
        ikcpcb* kcp() const { return kcp_; };
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
