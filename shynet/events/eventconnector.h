#ifndef SHYNET_EVENTS_EVENTCONNECTOR_H
#define SHYNET_EVENTS_EVENTCONNECTOR_H

#include "shynet/events/eventbase.h"
#include "shynet/events/eventbuffer.h"
#include "shynet/events/eventbufferssl.h"

namespace shynet {
namespace events {
    /**
     * @brief 连接器
    */
    class EventConnector : public Nocopy {
    public:
        /**
         * @brief 与服务器连接断开原因
        */
        enum class CloseType {
            /**
             * @brief 客户端主动断开
            */
            CLIENT_CLOSE,
            /**
             * @brief 服务区主动断开
            */
            SERVER_CLOSE,
            /**
             * @brief 连接服务器失败
            */
            CONNECT_FAIL,
            /**
             * @brief 与服务器心跳超时
            */
            TIMEOUT_CLOSE,
        };

        /**
         * @brief 连接器
         * @param base 反应堆
        */
        explicit EventConnector(std::shared_ptr<EventBase> base);
        ~EventConnector();

        /**
         * @brief 获取关联的反应堆
         * @return 反应堆
        */
        std::shared_ptr<EventBase> base() const;
        /**
         * @brief 获取关联的管理io缓冲
         * @return 管理io缓冲
        */
        std::shared_ptr<EventBuffer> buffer() const;
        /**
         * @brief 获取关联的dns对象
         * @return dns对象
        */
        evdns_base* dnsbase() const;
        /**
         * @brief 获取关联的socket文件描述符
         * @return socket文件描述符
        */
        evutil_socket_t fd() const;
        /**
         * @brief 通过地址连接服务器
         * @param address 地址对象
         * @param addrlen 地址对象大小
         * @return 0成功,-1失败
        */
        int connect(struct sockaddr* address, int addrlen);
        /**
         * @brief 通过域名连接服务器
         * @param hostname 域名
         * @param port 端口
         * @return 0成功,-1失败
        */
        int connect_hostname(const char* hostname, int port);
        /**
         * @brief 通过地址连接服务器(SSL)
         * @param address 地址对象
         * @param addrlen 地址对象大小
         * @return 0成功,-1失败
        */
        int connect_ssl(struct sockaddr* address, int addrlen);
        /**
         * @brief 通过域名连接服务器(SSL)
         * @param hostname 域名
         * @param port 端口
         * @return 0成功,-1失败
        */
        int connect_hostname_ssl(const char* hostname, int port);
        /**
         * @brief socket数据已经保存到管理io缓冲,可以读取回调
         * @param bev 管理io缓冲
        */
        virtual void input(const std::shared_ptr<EventBuffer> bev) = 0;
        /**
         * @brief 指定数据已经完成发送到管理io缓冲回调
         * @param bev 管理io缓冲
        */
        virtual void output(const std::shared_ptr<EventBuffer> bev) = 0;
        /**
         * @brief 连接成功回调
         * @param bev 管理io缓冲
        */
        virtual void success(const std::shared_ptr<EventBuffer> bev) = 0;
        /**
         * @brief 连接失败回调
         * @param bev 管理io缓冲
        */
        virtual void error(const std::shared_ptr<EventBuffer> bev) = 0;
        /**
         * @brief 与服务器连接断开回调
         * @param active 断开原因
        */
        virtual void close(CloseType active) = 0;

    private:
        /**
         * @brief 管理io缓冲
        */
        std::shared_ptr<EventBuffer> buffer_ = nullptr;
        /**
         * @brief ssl的管理io缓冲
        */
        std::shared_ptr<EventBufferSsl> bufferssl_ = nullptr;
        /**
         * @brief 反应堆
        */
        std::shared_ptr<EventBase> base_ = nullptr;
        /**
         * @brief dns对象
        */
        evdns_base* dnsbase_ = nullptr;
        /**
         * @brief ssl上下文
        */
        SSL_CTX* ctx_ = nullptr;
    };
}
}

#endif
