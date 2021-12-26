#ifndef SHYNET_EVENTS_EVENTHANDLER_H
#define SHYNET_EVENTS_EVENTHANDLER_H

#include "shynet/events/eventbase.h"

namespace shynet {
namespace events {

    /**
     * @brief 事件处理器
    */
    class EventHandler : public Nocopy {
    public:
        /**
         * @brief 构造
        */
        EventHandler();
        /**
         * @brief 构造
         * @param base 反应堆
         * @param fd socket文件描述符
         * @param what 事件处理器标识 EV_TIMEOUT，EV_READ，EV_WRITE，EV_SIGNAL，EV_PERSIST
        */
        EventHandler(std::shared_ptr<EventBase> base, evutil_socket_t fd, short what);
        ~EventHandler();
        /**
         * @brief 设置原生的event,
         * 必须保证原生的event没有通过EventHandler构造产生
         * @param base 反应堆
         * @param fd 文件描述符
         * @param what 事件处理器标识 EV_TIMEOUT，EV_READ，EV_WRITE，EV_SIGNAL，EV_PERSIST
        */
        void set_event(std::shared_ptr<EventBase> base, evutil_socket_t fd, short what);
        /**
         * @brief 获取原生的event
        */
        struct event* event() const;
        /**
         * @brief 获取socket文件描述符
         * @return socket文件描述符
        */
        evutil_socket_t fd() const;
        /**
         * @brief 获取反应堆
         * @return 反应堆
        */
        std::shared_ptr<EventBase> base() const;
        /**
         * @brief 获取设置事件处理器标识
         * @return EV_TIMEOUT，EV_READ，EV_WRITE，EV_SIGNAL，EV_PERSIST
        */
        short what() const;
        /**
         * @brief EV_READ回调，可以读数据回调
         * @param fd 文件描述符
        */
        virtual void input(int fd) {};
        /**
         * @brief EV_WRITE回调，指定数据已经发送回调
         * @param fd 文件描述符
        */
        virtual void output(int fd) {};
        /**
         * @brief EV_SIGNAL回调
         * @param fd 信号编号
        */
        virtual void signal(int fd) {};
        /**
         * @brief EV_TIMEOUT回调
         * @param fd -1
        */
        virtual void timeout(int fd) {};

    private:
        /**
         * @brief 原生事件
        */
        struct event* event_ = nullptr;
        /**
         * @brief 反应器
        */
        std::shared_ptr<EventBase> base_ = nullptr;
    };
}
}

#endif
