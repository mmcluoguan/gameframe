#ifndef SHYNET_EVENTS_EVENTBUFFERSSL_H
#define SHYNET_EVENTS_EVENTBUFFERSSL_H

#include "shynet/events/eventbuffer.h"
#include <event2/bufferevent_ssl.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

namespace shynet {
namespace events {
    /**
     * @brief ssl�Ĺ���io����
    */
    class EventBufferSsl final : public EventBuffer {
    public:
        /**
         * @brief ����
         * @param base ��Ӧ��
         * @param fd socket�ļ�������
         * @param state ״ֵ̬
         BUFFEREVENT_SSL_OPEN,BUFFEREVENT_SSL_CONNECTING,BUFFEREVENT_SSL_ACCEPTING
         * @param options �������BEV_OPT_CLOSE_ON_FREE��
        BEV_OPT_THREADSAFE��BEV_OPT_DEFER_CALLBACKS��BEV_OPT_UNLOCK_CALLBACKS
         * @param ctx ssl������
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
