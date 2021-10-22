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
    /// <summary>
    /// 反应堆
    /// </summary>
    class EventBase final : public Nocopy {
    public:
        EventBase();
        explicit EventBase(const EventConfig* cfg);
        ~EventBase();

        event_base* base() const;

        /// <summary>
        /// 执行事件检测
        /// </summary>
        /// <returns>0成功,-1失败</returns>
        int dispatch() const;
        /// <summary>
        /// 执行事件检测
        /// </summary>
        /// <param name="flag">EVLOOP_ONCE,EVLOOP_NONBLOCK,EVLOOP_NO_EXIT_ON_EMPTY
        /// <returns>0成功,-1失败</returns>
        int loop(int flag = 0) const;
        /// <summary>
        /// 让EventBase在给定时间之后停止循环.如果tv参数NULL,Reactor会立即停止循环,没有延时.
        /// 如EventBase当前正在执行任何激活事件的回调,则回调会继续运行,直到运行完所有激活事件的回调之才退出.
        /// </summary>
        /// <param name="tv"></param>
        /// <returns>0成功,-1失败</returns>
        int loopexit(const timeval* tv = nullptr) const;
        /// <summary>
        /// 让EventBase立即退出循环.它与 loopexit(NULL)的不同在于,
        /// 如果EventBase当前正在执行激活事件的回调,它将在执行完当前正在处理的事件后立即退出.
        /// </summary>
        /// <returns>0成功,-1失败</returns>
        int loopbreak() const;
        /// <summary>
        /// 因为调用 loopexit()而退出的时候返回 true,否则返回 false
        /// </summary>
        /// <returns></returns>
        bool gotexit() const;
        /// <summary>
        /// 因为调用 loopbreak()而退出的时候返回 true,否则返回 false
        /// </summary>
        /// <returns></returns>
        bool gotbreak() const;

        /// <summary>
        /// 添加事件处理器到反应堆中
        /// </summary>
        /// <param name="handler">事件处理器</param>
        /// <param name="tv">超时时间</param>
        /// <returns>0成功,-1失败</returns>
        int addevent(const std::shared_ptr<EventHandler> handler, const timeval* tv) const;
        /// <summary>
        /// 从反应堆中移除事件处理器
        /// </summary>
        /// <param name="handler">事件处理器</param>
        /// <returns>0成功,-1失败</returns>
        int delevent(const std::shared_ptr<EventHandler> handler) const;

        /// <summary>
        /// 创建成对的EventBuffer到pair数组,
        /// 创建的对象需要外部释放
        /// </summary>
        /// <param name="pair">pair[0]和pair[1]必须为null</param>
        void make_pair_buffer(std::shared_ptr<EventBuffer> pair[2]);

        static int usethread();
        static void event_shutdown();
        static void initssl();
        static void cleanssl();

    private:
        event_base* base_ = nullptr;
    };
}
}

#endif
