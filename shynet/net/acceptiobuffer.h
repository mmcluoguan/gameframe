#ifndef SHYNET_NET_ACCEPTIOBUFFER_H
#define SHYNET_NET_ACCEPTIOBUFFER_H

#include "shynet/events/eventbufferssl.h"
#include "shynet/net/acceptnewfd.h"

namespace shynet {
namespace net {
    /**
     * @brief 接收客户端数据管理io缓冲处理
    */
    class AcceptIoBuffer : public events::EventBuffer {
    public:
        /**
         * @brief 构造
         * @param base 反应器 
         * @param fd socket文件描述符
         * @param enable_ssl 是否启用ssl
         * @param ctx ssl上下文
        */
        AcceptIoBuffer(std::shared_ptr<events::EventBase> base,
            evutil_socket_t fd, bool enable_ssl = false, SSL_CTX* ctx = nullptr);
        ~AcceptIoBuffer() = default;

        /**
         * @brief 获取连接到服务器的客户端数据处理器
         * @return 连接到服务器的客户端数据处理器
        */
        std::weak_ptr<AcceptNewFd> newfd() const;
        /**
         * @brief 设置连接到服务器的客户端数据处理器
         * @param newfd 连接到服务器的客户端数据处理器
        */
        void set_newfd(std::weak_ptr<AcceptNewFd> newfd);

        /**
         * @brief socket数据已经保存到管理io缓冲,可以读取回调
        */
        void io_readcb();
        /**
         * @brief 指定数据已经完成发送到管理io缓冲回调
        */
        void io_writecb();
        /**
         * @brief socket发生事件回调
         * @param events BEV_EVENT_READING,	
         BEV_EVENT_WRITING,
         BEV_EVENT_EOF,
         BEV_EVENT_ERROR,
         BEV_EVENT_TIMEOUT,
         BEV_EVENT_CONNECTED,
         BEV_EVENT_READING 
        */
        void io_eventcb(short events);

    private:
        /**
         * @brief 管理io缓冲
        */
        std::shared_ptr<events::EventBuffer> iobuf_;
        /**
         * @brief 接收客户端数据处理器
        */
        std::weak_ptr<AcceptNewFd> newfd_;
    };
}
}

#endif
