#pragma once
#include "shynet/events/EventBase.h"
#include "shynet/events/Streambuff.h"
#include <event2/bufferevent.h>

namespace shynet {
	namespace events {
		/// <summary>
		/// 流事件处理器
		/// </summary>
		class EventBuffer : public Nocopy {
		public:
			/// <summary>
			/// 创建socket流事件处理器
			/// </summary>
			/// <param name="base">反应堆</param>
			/// <param name="fd">文件操作符</param>
			/// <param name="options">
			/// BEV_OPT_CLOSE_ON_FREE，
			/// BEV_OPT_THREADSAFE，
			/// BEV_OPT_DEFER_CALLBACKS，
			/// BEV_OPT_UNLOCK_CALLBACKS
			/// </param>
			EventBuffer(std::shared_ptr<EventBase> base, evutil_socket_t fd, int options);
			/// <summary>
			/// 引用原生流事件处理器
			/// </summary>
			/// <param name="buffer"></param>
			explicit EventBuffer(bufferevent* buffer, bool del = false);
			explicit EventBuffer(std::shared_ptr<EventBase> base);
			~EventBuffer();

			std::shared_ptr<EventBase> base() const;
			bufferevent* buffer() const;
			void set_buffer(bufferevent* buffer);

			int fd() const;

			/// <summary>
			/// 设置回调
			/// </summary>
			/// <param name="readcb">可读回调</param>
			/// <param name="writecb">可写回调</param>
			/// <param name="eventcb">事件回调</param>
			/// <param name="cbarg">外带参数</param>
			void setcb(bufferevent_data_cb readcb, bufferevent_data_cb writecb, bufferevent_event_cb eventcb, void* cbarg) const;

			/// <summary>
			/// 启动操作
			/// </summary>
			/// <param name="what">EV_READ，EV_WRITE</param>
			void enabled(short what) const;
			/// <summary>
			/// 禁用操作
			/// </summary>
			/// <param name="what">EV_READ，EV_WRITE</param>
			void disable(short what) const;
			/// <summary>
			/// 获取已设置操作操作
			/// </summary>
			/// <returns>EV_READ，EV_WRITE</returns>
			short what() const;

			/// <summary>
			/// 从EventBuffer输入缓冲区中读取指定size大小的数据到data,
			/// 并且把读出的数据从EventBuffer输入缓冲区中移除
			/// </summary>
			/// <param name="data"></param>
			/// <param name="size"></param>
			/// <returns>实际移除的字节数</returns>
			size_t read(void* data, size_t size) const;
			/// <summary>
			/// 从EventBuffer中读取全部数据到buffer,并且清空EventBuffer输入缓冲区的所有数据
			/// </summary>
			/// <param name="buffer"></param>
			/// <returns>0成功,-1失败</returns>
			int read(const std::shared_ptr<Streambuff> buffer) const;
			/// <summary>
			/// 写入指定size大小的数据到EventBuffer输出缓冲区中
			/// </summary>
			/// <param name="data"></param>
			/// <param name="size"></param>
			/// <returns>0成功,-1失败</returns>
			int write(const void* data, size_t size) const;
			/// <summary>
			/// 写buffer数据到EventBuffer输出缓冲区中,并且清空buffer的所有数据
			/// </summary>
			/// <param name="buffer"></param>
			/// <returns>0成功,-1失败</returns>
			int write(const std::shared_ptr<Streambuff> buffer) const;

			std::shared_ptr<Streambuff> inputbuffer() const;
			std::shared_ptr<Streambuff> outputbuffer() const;
		private:
			bufferevent* buffer_ = nullptr;
			std::shared_ptr<EventBase> base_ = nullptr;
			bool delflag_ = true;
		};
	}
}
