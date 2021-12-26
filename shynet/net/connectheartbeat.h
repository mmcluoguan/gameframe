#ifndef SHYNET_NET_CONNECTHEARTBEAT_H
#define SHYNET_NET_CONNECTHEARTBEAT_H

#include "shynet/net/timerevent.h"

namespace shynet {
namespace net {
    /**
     * @brief ���շ��������ݴ�����
    */
    class ConnectEvent;

    /**
     * @brief �ͻ��˼���������������ʱ������
    */
    class ConnectHeartbeat : public TimerEvent {
    public:
        /**
         * @brief ����
         * @param cnv ���շ��������ݴ�����
         * @param val ��ʱ���ʱ��ֵ
        */
        ConnectHeartbeat(std::weak_ptr<ConnectEvent> cnv, const struct timeval val);
        ~ConnectHeartbeat() = default;

        /**
         * @brief �ͻ��˼���������������ʱ������
        */
        void timeout() override;

    private:
        /**
         * @brief ���շ��������ݴ�����
        */
        std::weak_ptr<ConnectEvent> cnv_;
    };

}
}

#endif
