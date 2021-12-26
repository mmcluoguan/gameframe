#ifndef SHYNET_EVENTS_EVENTBASE_H
#define SHYNET_EVENTS_EVENTBASE_H

#include "shynet/events/eventconfig.h"
#include <event2/event.h>
#include <event2/thread.h>
#include <memory>

namespace shynet {
namespace events {

    class EventHandler;
    class EventBuffer;
    /**
     * @brief 反应堆
    */
    class EventBase final : public Nocopy {
    public:
        /**
         * @brief 构造
        */
        EventBase();
        /**
         * @brief 构造
         * @param cfg 配置
        */
        explicit EventBase(const EventConfig* cfg);
        ~EventBase();

        /**
         * @brief event_base包装
         * @return event_base 
        */
        event_base* base() const;
        /**
         * @brief 执行事件检测派发
         * @return 0成功,-1失败
        */
        int dispatch() const;
        /**
         * @brief 事件循环
         * @param flag EVLOOP_ONCE,EVLOOP_NONBLOCK,EVLOOP_NO_EXIT_ON_EMPTY
         * @return 0成功,-1失败
        */
        int loop(int flag = 0) const;
        /**
         * @brief 让EventBase在给定时间之后停止循环.如果tv参数NULL,Reactor会立即停止循环,没有延时.
        如EventBase当前正在执行任何激活事件的回调,则回调会继续运行,直到运行完所有激活事件的回调之才退出.
         * @param tv 时间
         * @return 0成功,-1失败
        */
        int loopexit(const timeval* tv = nullptr) const;
        /**
         * @brief 让EventBase立即退出循环.它与 loopexit(NULL)的不同在于,
        如果EventBase当前正在执行激活事件的回调,它将在执行完当前正在处理的事件后立即退出.
         * @return 0成功,-1失败
        */
        int loopbreak() const;
        /**
         * @brief 因为调用 loopexit()而退出的时候返回 true,否则返回 false
         * @return 因为调用 loopexit()而退出的时候返回 true,否则返回 false
        */
        bool gotexit() const;
        /**
         * @brief 因为调用 loopbreak()而退出的时候返回 true,否则返回 false
         * @return 因为调用 loopbreak()而退出的时候返回 true,否则返回 false
        */
        bool gotbreak() const;
        /**
         * @brief 添加事件处理器到反应堆中
         * @param handler 事件处理器
         * @param tv 等待事件的最长时间，或NULL永远等待 
         * @return 0成功,-1失败
        */
        int addevent(const std::shared_ptr<EventHandler> handler, const timeval* tv) const;
        /**
         * @brief 添加事件处理器到反应堆中
         * @param handler 事件处理器
         * @param tv 等待事件的最长时间，或NULL永远等待 
         * @return 0成功,-1失败
        */
        int addevent(const EventHandler* handler, const timeval* tv) const;
        /**
         * @brief 从反应堆中移除事件处理器
         * @param handler 事件处理器
         * @return 0成功,-1失败
        */
        int delevent(const std::shared_ptr<EventHandler> handler) const;
        /**
         * @brief 从反应堆中移除事件处理器
         * @param handler 事件处理器
         * @return 0成功,-1失败
        */
        int delevent(const EventHandler* handler) const;
        /**
         * @brief 创建成对的EventBuffer到pair数组,创建的对象需要外部释放
         * @param pair pair[0]和pair[1]必须为null
        */
        void make_pair_buffer(std::shared_ptr<EventBuffer> pair[2]);
        /**
         * @brief 开启反应堆线程安全,并使反应堆内存管理使用jemalloc
         * @return 
        */
        static int usethread();
        /**
         * @brief 释放反应堆内存
        */
        static void event_shutdown();
        /**
         * @brief 初始化ssl
        */
        static void initssl();
        /**
         * @brief 释放ssl内存
        */
        static void cleanssl();

    private:
        /**
         * @brief 用于保存 Libevent 调度循环的信息和状态的结构
        */
        event_base* base_ = nullptr;
    };
}
}

#endif
