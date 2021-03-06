#ifndef SHYNET_EVENTS_EVENTBUFFERFILTER_H
#define SHYNET_EVENTS_EVENTBUFFERFILTER_H

#include "shynet/events/eventbuffer.h"

namespace shynet {
namespace events {
    /**
     * @brief 管理io缓冲过滤器
    */
    class EventBufferFilter final : public Nocopy {
    public:
        /**
         * @brief 构造
         * @param source 管理io缓冲
        */
        explicit EventBufferFilter(const std::shared_ptr<EventBuffer> source);
        ~EventBufferFilter();

        /**
         * @brief 所有通过底层bufferevent接收的数据在到达过滤bufferevent之前都会经过"输入"过滤器的转换
         * @param source 原始数据
         * @param dest 原始数据过滤后的数据
         * @return 如果成功向目标缓冲区写入了任何数据,过滤器函数应该返回BEV_OK;
        如果没有获得更多的输入,或者不使用不同的清空(flush)模式,就不能向目标缓冲区写入更多的数据,
        则应该返回BEV_NEED_MORE;
        如果过滤器上发生了不可恢复的错误,则应该返 回BEV_ERROR
        */
        virtual bufferevent_filter_result infilter(const std::shared_ptr<Streambuff> source,
            const std::shared_ptr<Streambuff> dest);

        /**
         * @brief 所有通过底层bufferevent发送的数据在被发送到底层bufferevent之前都会经过"输出"过滤器的转换
         * @param source 原始数据
         * @param dest 原始数据过滤后的数据
         * @return 如果成功向目标缓冲区写入了任何数据,过滤器函数应该返回BEV_OK;
        如果没有获得更多的输入,或者不使用不同的清空(flush)模式,就不能向目标缓冲区写入更多的数据,
        则应该返回BEV_NEED_MORE;
        如果过滤器上发生了不可恢复的错误,则应该返 回BEV_ERROR
        */
        virtual bufferevent_filter_result outfilter(const std::shared_ptr<Streambuff> source,
            const std::shared_ptr<Streambuff> dest);

    private:
        /**
         * @brief 原生管理io缓冲
        */
        bufferevent* buffer_ = nullptr;
    };
}
}

#endif
