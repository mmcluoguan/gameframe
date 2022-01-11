#ifndef FRMPUB_RESPONSETIMER_H
#define FRMPUB_RESPONSETIMER_H

#include "shynet/net/ipaddress.h"
#include "shynet/net/timerevent.h"

namespace frmpub {
/**
 * @brief 消息数据封包过滤器
*/
class FilterData;
/**
 * @brief 客户端请求服务器消息,服务器返回计时处理器
*/
class ResponseTimer : public net::TimerEvent {
public:
    /**
     * @brief 构造
     * @param val 超时相对时间值
     * @param filter 消息数据封包过滤器
     * @param msgid_c 请求消息id
     * @param msgid_s 回应消息id
    */
    ResponseTimer(const struct timeval val,
        std::weak_ptr<FilterData> filter,
        int msgid_c, int msgid_s);
    ~ResponseTimer() = default;
    /**
    * @brief 计时器超时后,在工作线程中处理超时回调
    */
    void timeout() override;

private:
    /**
     * @brief 消息数据封包过滤器
    */
    std::weak_ptr<FilterData> filter_;
    /**
     * @brief 请求消息
    */
    int msgid_c_;
    /**
     * @brief 回应消息id
    */
    int msgid_s_;
};
}

#endif
