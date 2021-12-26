#ifndef SHYNET_NET_CONNECTHEARTBEAT_H
#define SHYNET_NET_CONNECTHEARTBEAT_H

#include "shynet/net/timerevent.h"

namespace shynet {
namespace net {
    /**
     * @brief 接收服务器数据处理器
    */
    class ConnectEvent;

    /**
     * @brief 客户端检测服务器心跳包超时处理器
    */
    class ConnectHeartbeat : public TimerEvent {
    public:
        /**
         * @brief 构造
         * @param cnv 接收服务器数据处理器
         * @param val 超时相对时间值
        */
        ConnectHeartbeat(std::weak_ptr<ConnectEvent> cnv, const struct timeval val);
        ~ConnectHeartbeat() = default;

        /**
         * @brief 客户端检测服务器心跳包超时处理处理
        */
        void timeout() override;

    private:
        /**
         * @brief 接收服务器数据处理器
        */
        std::weak_ptr<ConnectEvent> cnv_;
    };

}
}

#endif
