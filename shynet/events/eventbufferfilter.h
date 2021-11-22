#ifndef SHYNET_EVENTS_EVENTBUFFERFILTER_H
#define SHYNET_EVENTS_EVENTBUFFERFILTER_H

#include "shynet/events/eventbuffer.h"

namespace shynet {
namespace events {
    class EventBufferFilter final : public Nocopy {
    public:
        explicit EventBufferFilter(const std::shared_ptr<EventBuffer> source);
        ~EventBufferFilter();

        /// <summary>
        /// 所有通过底层bufferevent接收的数据在到达过滤bufferevent之前都会经过"输入"过滤器的转换
        /// </summary>
        /// <param name="source"></param>
        /// <param name="dest"></param>
        /// <returns>如果成功向目标缓冲区写入了任何数据,过滤器函数应该返回BEV_OK;
        /// 如果没有获得更多的输入,或者不使用不同的清空(flush)模式,就不能向目标缓冲区写入更多的数据,
        /// 则应该返回BEV_NEED_MORE;
        /// 如果过滤器上发生了不可恢复的错误,则应该返 回BEV_ERROR.</returns>
        virtual bufferevent_filter_result infilter(const std::shared_ptr<Streambuff> source,
            const std::shared_ptr<Streambuff> dest);
        /// <summary>
        /// 所有通过底层bufferevent发送的数据在被发送到底层bufferevent之前都会经过"输出"过滤器的转换
        /// </summary>
        /// <param name="source"></param>
        /// <param name="dest"></param>
        /// <returns>如果成功向目标缓冲区写入了任何数据,过滤器函数应该返回BEV_OK;
        /// 如果没有获得更多的输入,或者不使用不同的清空(flush)模式,就不能向目标缓冲区写入更多的数据,
        /// 则应该返回BEV_NEED_MORE;
        /// 如果过滤器上发生了不可恢复的错误,则应该返 回BEV_ERROR.</returns>
        virtual bufferevent_filter_result outfilter(const std::shared_ptr<Streambuff> source,
            const std::shared_ptr<Streambuff> dest);

    private:
        bufferevent* buffer_ = nullptr;
    };
}
}

#endif
