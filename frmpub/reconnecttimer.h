#ifndef FRMPUB_RECONNECTTIMER_H
#define FRMPUB_RECONNECTTIMER_H

#include "shynet/net/connectreactormgr.h"
#include "shynet/net/ipaddress.h"
#include "shynet/net/timerreactormgr.h"
#include "shynet/utils/elapsed.h"

namespace frmpub {
/**
 * @brief 自动重连服务器计时处理器
 * @tparam T 服务器连接器类型
*/
template <class T>
class ReConnectTimer : public net::TimerEvent {
public:
    /**
     * @brief 构造
     * @param connect_addr 连接的服务器地址
     * @param val 超时相对时间值
    */
    ReConnectTimer(std::shared_ptr<net::IPAddress> connect_addr, const struct timeval val)
        : net::TimerEvent(val, EV_TIMEOUT)
    {
        connect_addr_ = connect_addr;
    }
    ~ReConnectTimer() = default;
    /**
     * @brief 计时器超时后,在工作线程中处理超时回调
    */
    void timeout() override
    {
        auto cb = [&]() {
            shynet::utils::Singleton<net::TimerReactorMgr>::instance().remove(timerid());
            std::shared_ptr<T> reconnect = std::make_shared<T>(connect_addr_);
            shynet::utils::Singleton<net::ConnectReactorMgr>::instance().add(reconnect);
            reconnect.reset();
        };
#ifdef USE_DEBUG
        shynet::utils::elapsed("工作线程计时单任务执行 ReConnectTimer");
        return cb();
#else
        return cb();
#endif
    }

private:
    /**
     * @brief 连接的服务器地址
     * @tparam T 
    */
    std::shared_ptr<net::IPAddress> connect_addr_;
};
}

#endif
