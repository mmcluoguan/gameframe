#ifndef SHYNET_NET_ACCEPTIOBUFFER_H
#define SHYNET_NET_ACCEPTIOBUFFER_H

#include "shynet/events/eventbufferssl.h"
#include "shynet/net/acceptnewfd.h"

namespace shynet {
namespace net {
    /**
     * @brief ���տͻ������ݹ���io���崦��
    */
    class AcceptIoBuffer : public events::EventBuffer {
    public:
        /**
         * @brief ����
         * @param base ��Ӧ�� 
         * @param fd socket�ļ�������
         * @param enable_ssl �Ƿ�����ssl
         * @param ctx ssl������
        */
        AcceptIoBuffer(std::shared_ptr<events::EventBase> base,
            evutil_socket_t fd, bool enable_ssl = false, SSL_CTX* ctx = nullptr);
        ~AcceptIoBuffer() = default;

        /**
         * @brief ��ȡ���ӵ��������Ŀͻ������ݴ�����
         * @return ���ӵ��������Ŀͻ������ݴ�����
        */
        std::weak_ptr<AcceptNewFd> newfd() const;
        /**
         * @brief �������ӵ��������Ŀͻ������ݴ�����
         * @param newfd ���ӵ��������Ŀͻ������ݴ�����
        */
        void set_newfd(std::weak_ptr<AcceptNewFd> newfd);

        /**
         * @brief socket�����Ѿ����浽����io����,���Զ�ȡ�ص�
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
         * @brief ���տͻ������ݴ�����
        */
        std::weak_ptr<AcceptNewFd> newfd_;
    };
}
}

#endif
