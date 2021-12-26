#ifndef SHYNET_NET_TIMEREVENT_H
#define SHYNET_NET_TIMEREVENT_H

#include "shynet/events/eventhandler.h"

namespace shynet {
namespace net {
    /**
     * @brief 计时器处理器
    */
    class TimerEvent : public events::EventHandler {
    public:
        /**
         * @brief 构造
         * @param val 超时相对时间值
         * @param what 事件处理器标识 EV_TIMEOUT，EV_PERSIST
        */
        TimerEvent(const struct timeval val, short what);
        ~TimerEvent() = default;

        /**
         * @brief 获取超时相对时间值
        */
        const struct timeval& val() const { return val_; }
        /**
         * @brief 重新设置超时时间，如果超时未触发，则延迟指定时间
         * @param t 超时相对时间值
        */
        void set_val(const struct timeval& t);

        /**
         * @brief 获取事件处理器标识
         * @return 事件处理器标识 EV_TIMEOUT，EV_PERSIST
        */
        short what() const { return what_; }

        /**
         * @brief 获取计时器id
         * @return 计时器id
        */
        int timerid() const { return timerid_; }
        /**
         * @brief 设置计时器id
         * @param timerid 计时器id
        */
        void set_timerid(int timerid) { timerid_ = timerid; }

        /**
         * @brief 超时回调,创建计时器任务投递到工作线程中
         * @param fd -1
        */
        void timeout(int fd) override;

        /**
         * @brief 计时器超时后,在工作线程中处理超时回调
        */
        virtual void timeout() = 0;

    private:
        /**
         * @brief 超时相对时间值
        */
        struct timeval val_;
        /**
         * @brief 事件处理器标识 EV_TIMEOUT，EV_PERSIST
        */
        short what_ = 0;
        /**
         * @brief 计时器id
        */
        int timerid_ = 0;
    };

}
}

#endif
