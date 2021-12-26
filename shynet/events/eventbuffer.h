#ifndef SHYNET_EVENTS_EVENTBUFFER_H
#define SHYNET_EVENTS_EVENTBUFFER_H

#include "shynet/events/eventbase.h"
#include "shynet/events/streambuff.h"
#include <event2/bufferevent.h>

namespace shynet {
namespace events {
    /**
     * @brief 管理io缓冲
    */
    class EventBuffer : public Nocopy {
    public:
        /**
         * @brief 构造
         * @param base 关联的反应堆
         * @param fd socket文件操作符
         * @param options 零个或多个BEV_OPT_CLOSE_ON_FREE，
        BEV_OPT_THREADSAFE，BEV_OPT_DEFER_CALLBACKS，BEV_OPT_UNLOCK_CALLBACKS
        */
        EventBuffer(std::shared_ptr<EventBase> base, evutil_socket_t fd, int options);
        /**
         * @brief 构造
         * @param buffer 引用管理原生io缓冲
         * @param del 析构时是否释放管理原生io缓冲
        */
        explicit EventBuffer(bufferevent* buffer, bool del = false);
        /**
         * @brief 构造
         * @param base 关联的反应堆
        */
        explicit EventBuffer(std::shared_ptr<EventBase> base);
        ~EventBuffer();
        /**
         * @brief 关联的反应堆
         * @return 关联的反应堆
        */
        std::shared_ptr<EventBase> base() const;
        /**
         * @brief 获取管理原生io缓冲
         * @return 管理原生io缓冲
        */
        bufferevent* buffer() const;
        /**
         * @brief 设置管理原生io缓冲
         * @param buffer 管理原生io缓冲
        */
        void set_buffer(bufferevent* buffer);
        /**
         * @brief 获取管理io缓冲关联的fd
         * @return 管理io缓冲关联的fd
        */
        int fd() const;
        /**
         * @brief 设置回调
         * @param readcb socket数据已经保存到管理io缓冲,可以读取回调
         * @param writecb 指定数据已经完成发送到管理io缓冲回调
         * @param eventcb socket发生
         BEV_EVENT_READING,	
         BEV_EVENT_WRITING,
         BEV_EVENT_EOF,
         BEV_EVENT_ERROR,
         BEV_EVENT_TIMEOUT,
         BEV_EVENT_CONNECTED,
         BEV_EVENT_READING 事件回调
         * @param cbarg 外带参数
        */
        void setcb(bufferevent_data_cb readcb, bufferevent_data_cb writecb, bufferevent_event_cb eventcb, void* cbarg) const;
        /**
         * @brief 启用管理io缓冲事件
         * @param what EV_READ|EV_WRITE 的任意组合
        */
        void enabled(short what) const;
        /**
         * @brief 禁用管理io缓冲事件
         * @param what EV_READ|EV_WRITE 的任意组合
        */
        void disable(short what) const;
        /**
         * @brief 获取已设置操作操作
         * @return EV_READ|EV_WRITE 的任意组合
        */
        short what() const;
        /**
         * @brief 从EventBuffer输入缓冲区中读取指定size大小的数据到data,
         并且把读出的数据从EventBuffer输入缓冲区中移除
         * @param data 指向将存储数据的缓冲区的指针
         * @param size 数据缓冲区的大小，以字节为单位
         * @return 读取的数据量，以字节为单位
        */
        size_t read(void* data, size_t size) const;
        /**
         * @brief 从EventBuffer中读取全部数据到buffer,
         并且清空EventBuffer输入缓冲区的所有数据
         * @param buffer io缓冲
         * @return 0成功,-1失败
        */
        int read(const std::shared_ptr<Streambuff> buffer) const;
        /**
         * @brief 写入指定size大小的数据到EventBuffer输出缓冲区中,
         * 当关联描述符在它变得可用时自动写入
         * @param data 指向将存储数据的缓冲区的指针
         * @param size 数据缓冲区的大小，以字节为单位
         * @return 0成功,-1失败
        */
        int write(const void* data, size_t size) const;
        /**
         * @brief 写buffer数据到EventBuffer输出缓冲区中,并且清空buffer的所有数据,
         * 当关联描述符在它变得可用时自动写入
         * @param buffer io缓冲
         * @return 0成功,-1失败
        */
        int write(const std::shared_ptr<Streambuff> buffer) const;

        /**
         * @brief 获取关联的输入io缓冲
         * @return 关联的输入io缓冲
        */
        std::shared_ptr<Streambuff> inputbuffer() const;
        /**
         * @brief 获取关联的输出io缓冲
         * @return 关联的输出io缓冲
        */
        std::shared_ptr<Streambuff> outputbuffer() const;

    private:
        /**
         * @brief 原生管理io缓冲
        */
        bufferevent* buffer_ = nullptr;
        /**
         * @brief 反应堆
        */
        std::shared_ptr<EventBase> base_ = nullptr;
        /**
         * @brief 析构时是否释放管理原生io缓冲
        */
        bool delflag_ = true;
    };
}
}

#endif
