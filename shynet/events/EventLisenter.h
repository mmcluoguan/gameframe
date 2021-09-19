#pragma once
#include "shynet/events/EventBase.h"
#include <event2/listener.h>

namespace shynet {
	namespace events {
		class EventLisenter : public Nocopy {
		public:
			EventLisenter(std::shared_ptr<EventBase> base, const sockaddr* sa, int socklen);
			~EventLisenter();

			std::shared_ptr<EventBase> base() const;
			evutil_socket_t fd() const;

			int disable() const;
			int enable() const;

			/// <summary>
			/// 新连接
			/// </summary>
			/// <param name="fd"></param>
			/// <param name="address"></param>
			/// <param name="socklen"></param>
			virtual void input(evutil_socket_t fd, struct sockaddr* address, int socklen) = 0;
			/// <summary>
			/// 监听失败
			/// </summary>
			///<param name="bev"></param>
			virtual void error() = 0;
		private:
			std::shared_ptr<EventBase> base_ = nullptr;
			evconnlistener* listener_;
		};
	}
}
