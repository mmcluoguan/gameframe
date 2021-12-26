#ifndef SHYNET_NET_CONNECTIOBUFFER_H
#define SHYNET_NET_CONNECTIOBUFFER_H

#include "shynet/events/eventbufferssl.h"
#include "shynet/net/connectevent.h"

namespace shynet {
namespace net {
    /**
     * @brief 接收服务器数据管理io缓冲处理
    */
    class ConnectIoBuffer : public events::EventBuffer {
    public:
        /**
         * @brief 构造
         * @param base 反应器
         * @param enable_ssl 是否启用ssl
         * @param ctx ssl上下文
        */
        ConnectIoBuffer(std::shared_ptr<events::EventBase> base, bool enable_ssl = false, SSL_CTX* ctx = nullptr);
        ~ConnectIoBuffer() = default;

        /**
         * @brief 获取接收服务器数据处理器
         * @return 接收服务器数据处理器
        */
        std::weak_ptr<ConnectEvent> cnev() const;
        /**
         * @brief 设置接收服务器数据处理器
         * @param cnev 接收服务器数据处理器
        */
        void set_cnev(std::weak_ptr<ConnectEvent> cnev);

        /**
         * @brief  socket数据已经保存到管理io缓冲,可以读取回调
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
         * @brief 接收服务器数据处理器
        */
        std::weak_ptr<ConnectEvent> cnev_;
    };
}
}

#endif
