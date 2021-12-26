#ifndef FRMPUB_PINGTIMER_H
#define FRMPUB_PINGTIMER_H

#include "shynet/net/ipaddress.h"
#include "shynet/net/timerevent.h"

namespace frmpub {
/**
 * @brief ������������
*/
class Connector;
/**
 * @brief �ͻ����������������ʱ������
*/
class PingTimer : public net::TimerEvent {
public:
    /**
     * @brief ����
     * @param val ��ʱ���ʱ��ֵ
     * @param connector ������������
    */
    PingTimer(const struct timeval val,
        Connector* connector);
    ~PingTimer() = default;
    /**
    * @brief ��ʱ����ʱ��,�ڹ����߳��д���ʱ�ص�
    */
    void timeout() override;
    /**
     * @brief ��������ķ�����������
    */
    void clean_connector()
    {
        connector_ = nullptr;
    }

private:
    Connector* connector_;
};
}

#endif
