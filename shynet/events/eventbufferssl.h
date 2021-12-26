#ifndef SHYNET_EVENTS_EVENTBUFFERSSL_H
#define SHYNET_EVENTS_EVENTBUFFERSSL_H

#include "shynet/events/eventbuffer.h"
#include <event2/bufferevent_ssl.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

namespace shynet {
namespace events {
    /**
     * @brief ssl的管理io缓冲
    */
    class EventBufferSsl final : public EventBuffer {
    public:
        /**
         * @brief 构造
         * @param base 反应器
         * @param fd socket文件描述符
         * @param state 状态值
         BUFFEREVENT_SSL_OPEN,BUFFEREVENT_SSL_CONNECTING,BUFFEREVENT_SSL_ACCEPTING
         * @param options 零个或多个BEV_OPT_CLOSE_ON_FREE，
        BEV_OPT_THREADSAFE，BEV_OPT_DEFER_CALLBACKS，BEV_OPT_UNLOCK_CALLBACKS
         * @param ctx ssl上下文
        */
        EventBufferSsl(std::shared_ptr<EventBase> base,
            evutil_socket_t fd,
            bufferevent_ssl_state state,
            int options,
            SSL_CTX* ctx);
        ~EventBufferSsl();

    private:
        SSL* ssl_ = nullptr;
    };
}
}

#endif
