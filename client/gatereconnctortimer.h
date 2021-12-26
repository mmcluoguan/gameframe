#ifndef CLIENT_GATERECONNCTORTIMER_H
#define CLIENT_GATERECONNCTORTIMER_H

#include "client/gateconnector.h"
#include "shynet/net/ipaddress.h"
#include "shynet/net/timerevent.h"

namespace client {
/**
 * @brief 自动重连服务器计时处理器
*/
class GateReConnctorTimer : public net::TimerEvent {
public:
    /**
     * @brief 构造
     * @param connect_addr 连接的服务器地址
     * @param data 重连前的连接数据
     * @param val 超时相对时间值
    */
    GateReConnctorTimer(
        std::shared_ptr<net::IPAddress> connect_addr,
        std::shared_ptr<GateConnector::DisConnectData> data,
        const struct timeval val);
    ~GateReConnctorTimer();
    /**
     * @brief 计时器超时后,在工作线程中处理超时回调
    */
    void timeout() override;

private:
    /**
     * @brief 连接的服务器地址
    */
    std::shared_ptr<net::IPAddress> connect_addr_;
    /**
     * @brief 重连前的连接数据
    */
    std::shared_ptr<GateConnector::DisConnectData> data_;
};
}

#endif
