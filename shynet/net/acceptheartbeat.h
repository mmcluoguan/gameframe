#ifndef SHYNET_NET_ACCEPTHEARTBEAT_H
#define SHYNET_NET_ACCEPTHEARTBEAT_H

#include "shynet/net/timerreactormgr.h"

namespace shynet {
namespace net {
    /**
     * @brief 接收客户端数据处理器
    */
    class AcceptNewFd;

    /**
     * @brief 服务器检测客户端心跳包超时处理器
    */
    class AcceptHeartbeat : public TimerEvent {
    public:
        /**
         * @brief 构造
         * @param newfd 接收客户端数据处理器
         * @param val 超时相对时间值
        */
        AcceptHeartbeat(std::weak_ptr<AcceptNewFd> newfd, const struct timeval val);
        ~AcceptHeartbeat() = default;

        /**
         * @brief 服务器检测客户端心跳包超时处理
        */
        void timeout() override;

    private:
        /**
         * @brief 接收客户端数据处理器
        */
        std::weak_ptr<AcceptNewFd> newfd_;
    };

}
}

#endif
