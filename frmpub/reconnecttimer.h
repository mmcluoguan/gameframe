#ifndef FRMPUB_RECONNECTTIMER_H
#define FRMPUB_RECONNECTTIMER_H

#include "shynet/net/connectreactormgr.h"
#include "shynet/net/ipaddress.h"
#include "shynet/net/timerreactormgr.h"

namespace frmpub {
/**
 * @brief �Զ�������������ʱ������
 * @tparam T ����������������
*/
template <class T>
class ReConnectTimer : public net::TimerEvent {
public:
    /**
     * @brief ����
     * @param connect_addr ���ӵķ�������ַ
     * @param val ��ʱ���ʱ��ֵ
    */
    ReConnectTimer(std::shared_ptr<net::IPAddress> connect_addr, const struct timeval val)
        : net::TimerEvent(val, EV_TIMEOUT)
    {
        connect_addr_ = connect_addr;
    }
    ~ReConnectTimer() = default;
    /**
     * @brief ��ʱ����ʱ��,�ڹ����߳��д���ʱ�ص�
    */
    void timeout() override
    {
        shynet::utils::Singleton<net::TimerReactorMgr>::instance().remove(timerid());
        std::shared_ptr<T> reconnect = std::make_shared<T>(connect_addr_);
        shynet::utils::Singleton<net::ConnectReactorMgr>::instance().add(reconnect);
        reconnect.reset();
    }

private:
    /**
     * @brief ���ӵķ�������ַ
     * @tparam T 
    */
    std::shared_ptr<net::IPAddress> connect_addr_;
};
}

#endif
