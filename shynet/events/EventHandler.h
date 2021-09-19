#pragma once
#include "shynet/events/EventBase.h"

namespace shynet
{
	namespace events
	{

		/// <summary>
		/// 事件处理器
		/// </summary>
		class EventHandler : public Nocopy
		{
		public:
			/// <summary>
			/// 创建事件处理器,必须调用event设置反应堆
			/// </summary>
			EventHandler();
			/// <summary>
			/// 创建事件处理器
			/// </summary>
			/// <param name="base">反应堆</param>
			/// <param name="fd">文件描述符</param>
			/// <param name="what">事件处理器标识 EV_TIMEOUT，EV_READ，EV_WRITE，EV_SIGNAL，EV_PERSIST</param>
			EventHandler(std::shared_ptr<EventBase> base, evutil_socket_t fd, short what);
			~EventHandler();
			/// <summary>
			/// 延迟创建底层的event，
			/// 必须保证底层的event没有通过EventHandler构造产生
			/// </summary>
			/// <param name="base"></param>
			/// <param name="fd"></param>
			/// <param name="what"></param>
			void event(std::shared_ptr<EventBase> base, evutil_socket_t fd, short what);
			struct event* event() const;
			evutil_socket_t fd() const;
			std::shared_ptr<EventBase> base() const;
			/// <summary>
			/// 事件处理器标识
			/// </summary>
			/// <returns>EV_TIMEOUT，EV_READ，EV_WRITE，EV_SIGNAL，EV_PERSIST</returns>
			short what() const;

			/// <summary>
			/// EV_READ回调，可以读数据回调
			/// </summary>
			/// <param name="fd">文件描述符</param>
			virtual void input(int fd) {};
			/// <summary>
			/// EV_WRITE回调，指定数据已经发送回调
			/// </summary>
			/// <param name="fd">文件描述符</param>
			virtual void output(int fd) {};
			/// <summary>
			/// EV_SIGNAL回调
			/// </summary>
			/// <param name="fd">信号编号</param>
			virtual void signal(int fd) {};
			/// <summary>
			/// EV_TIMEOUT回调
			/// </summary>
			/// <param name="fd">-1</param>
			virtual void timeout(int fd) {};
		private:
			struct event* event_ = nullptr;
			std::shared_ptr<EventBase> base_ = nullptr;
		};
	}
}
