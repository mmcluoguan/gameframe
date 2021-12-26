#ifndef SHYNET_NET_CONNECTIOBUFFER_H
#define SHYNET_NET_CONNECTIOBUFFER_H

#include "shynet/events/eventbufferssl.h"
#include "shynet/net/connectevent.h"

namespace shynet {
namespace net {
    /**
     * @brief ���շ��������ݹ���io���崦��
    */
    class ConnectIoBuffer : public events::EventBuffer {
    public:
        /**
         * @brief ����
         * @param base ��Ӧ��
         * @param enable_ssl �Ƿ�����ssl
         * @param ctx ssl������
        */
        ConnectIoBuffer(std::shared_ptr<events::EventBase> base, bool enable_ssl = false, SSL_CTX* ctx = nullptr);
        ~ConnectIoBuffer() = default;

        /**
         * @brief ��ȡ���շ��������ݴ�����
         * @return ���շ��������ݴ�����
        */
        std::weak_ptr<ConnectEvent> cnev() const;
        /**
         * @brief ���ý��շ��������ݴ�����
         * @param cnev ���շ��������ݴ�����
        */
        void set_cnev(std::weak_ptr<ConnectEvent> cnev);

        /**
         * @brief  socket�����Ѿ����浽����io����,���Զ�ȡ�ص�
        */
        void io_readcb();
        /**
         * @brief ָ�������Ѿ���ɷ��͵�����io����ص�
        */
        void io_writecb();
        /**
         * @brief socket�����¼��ص�
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
         * @brief ����io����
        */
        std::shared_ptr<events::EventBuffer> iobuf_;
        /**
         * @brief ���շ��������ݴ�����
        */
        std::weak_ptr<ConnectEvent> cnev_;
    };
}
}

#endif
