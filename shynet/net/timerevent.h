#ifndef SHYNET_NET_TIMEREVENT_H
#define SHYNET_NET_TIMEREVENT_H

#include "shynet/events/eventhandler.h"

namespace shynet {
namespace net {

    class TimerEvent : public events::EventHandler {
    public:
        /// <summary>
        /// 计时器事件
        /// </summary>
        /// <param name="val">相对时间值</param>
        /// <param name="what">事件处理器标识 EV_TIMEOUT，EV_PERSIST</param>
        TimerEvent(const struct timeval val, short what);
        ~TimerEvent();

        const struct timeval& val() const { return val_; }
        /// <summary>
        /// 重新设置超时时间，如果超时未触发，则延迟指定时间
        /// </summary>
        /// <param name="t"></param>
        void set_val(const struct timeval& t);

        /// <summary>
        /// 事件处理器标识 EV_TIMEOUT，EV_PERSIST
        /// </summary>
        /// <returns></returns>
        short what() const { return what_; }

        /*
        * 获取设置计时器id
        */
        int timerid() const { return timerid_; }
        void set_timerid(int timerid) { timerid_ = timerid; }

        /// <summary>
        /// EV_TIMEOUT回调
        /// </summary>
        /// <param name="fd">-1</param>
        void timeout(int fd) override;

        /// <summary>
        /// 计时器线程超时回调
        /// </summary>
        virtual void timeout() = 0;

    private:
        struct timeval val_;
        short what_ = 0;
        int timerid_ = 0;
    };

} // namespace net
} // namespace shynet

#endif
