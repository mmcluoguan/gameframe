#ifndef FRMPUB_PINGTIMER_H
#define FRMPUB_PINGTIMER_H

#include "shynet/net/ipaddress.h"
#include "shynet/net/timerevent.h"

namespace frmpub {
/**
 * @brief 服务器连接器
*/
class Connector;
/**
 * @brief 客户端与服务器心跳计时处理器
*/
class PingTimer : public net::TimerEvent {
public:
    /**
     * @brief 构造
     * @param val 超时相对时间值
     * @param connector 服务器连接器
    */
    PingTimer(const struct timeval val,
        Connector* connector);
    ~PingTimer() = default;
    /**
    * @brief 计时器超时后,在工作线程中处理超时回调
    */
    void timeout() override;
    /**
     * @brief 清除关联的服务器连接器
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
