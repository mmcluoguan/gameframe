#ifndef SHYNET_EVENTS_EVENTCONFIG_H
#define SHYNET_EVENTS_EVENTCONFIG_H

#include "shynet/basic.h"
#include <event2/event.h>

namespace shynet {
namespace events {

    /**
     * @brief 反应堆配置
    */
    class EventConfig final : public Nocopy {
    public:
        /**
         * @brief 构造
        */
        EventConfig();
        ~EventConfig();
        /**
         * @brief 设置反应堆特性
         * @param feature EV_FEATURE_ET,
         EV_FEATURE_O1,
         EV_FEATURE_FDS,
         EV_FEATURE_EARLY_CLOSE
         * @return 0成功,-1失败
        */
        int setfeatures(event_method_feature feature) const;
        /**
         * @brief 设置反应堆标识
         * @param flag EVENT_BASE_FLAG_NOLOCK,
        EVENT_BASE_FLAG_IGNORE_ENV,
        EVENT_BASE_FLAG_STARTUP_IOCP,
        EVENT_BASE_FLAG_NO_CACHE_TIME,
        EVENT_BASE_FLAG_EPOLL_USE_CHANGELIST,
        EVENT_BASE_FLAG_PRECISE_TIMER
         * @return 0成功,-1失败
        */
        int setflag(event_base_config_flag flag) const;
        /**
         * @brief 获取原生反应堆配置
         * @return 原生反应堆配置
        */
        event_config* config() const;

    private:
        /*
        * 原生反应堆配置
        */
        event_config* config_ = nullptr;
    };
}
}

#endif
