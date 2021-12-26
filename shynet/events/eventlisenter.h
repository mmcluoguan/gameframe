#ifndef SHYNET_EVENTS_EVENTLISENTER_H
#define SHYNET_EVENTS_EVENTLISENTER_H

#include "shynet/events/eventbase.h"
#include <event2/listener.h>

namespace shynet {
namespace events {
    /**
     * @brief socket侦听器
    */
    class EventLisenter : public Nocopy {
    public:
        /**
         * @brief 构造
         * @param base 反应器
         * @param sa 地址对象
         * @param socklen 地址大小
        */
        EventLisenter(std::shared_ptr<EventBase> base, const sockaddr* sa, int socklen);
        ~EventLisenter();

        /**
         * @brief 获取反应器
         * @return 反应器
        */
        std::shared_ptr<EventBase> base() const;
        /**
         * @brief 获取socket文件描述符
         * @return socket文件描述符
        */
        evutil_socket_t fd() const;

        /**
         * @brief 停止侦听器上的侦听
         * @return 0成功,-1失败
        */
        int disable() const;
        /**
         * @brief 重新开启侦听器上的侦听
         * @return 0成功,-1失败
        */
        int enable() const;
        /**
         * @brief 侦听到新链接回调
         * @param fd socket文件描述符
         * @param address 新链接的地址对象
         * @param socklen 新链接的地址对象大小
        */
        virtual void input(evutil_socket_t fd, struct sockaddr* address, int socklen) = 0;
        /**
         * @brief 侦听回调
        */
        virtual void error() = 0;

    private:
        /**
         * @brief 反应器
        */
        std::shared_ptr<EventBase> base_ = nullptr;
        /**
         * @brief 原生socket侦听器
        */
        evconnlistener* listener_;
    };
}
}

#endif
