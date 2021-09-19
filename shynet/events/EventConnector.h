#pragma once
#include "shynet/events/EventBase.h"
#include "shynet/events/EventBuffer.h"
#include "shynet/events/EventBufferSsl.h"

namespace shynet {
	namespace events {
		/// <summary>
		/// 连接器
		/// </summary>
		class EventConnector : public Nocopy {
		public:
			explicit EventConnector(std::shared_ptr<EventBase> base);
			~EventConnector();

			std::shared_ptr<EventBase> base() const;
			std::shared_ptr<EventBuffer> buffer() const;
			evdns_base* dnsbase() const;
			evutil_socket_t fd() const;

			/// <summary>
			/// 通过地址连接服务器
			/// </summary>
			/// <param name="address"></param>
			/// <param name="addrlen"></param>
			/// <returns>0成功,-1失败</returns>
			int connect(struct sockaddr* address, int addrlen);
			/// <summary>
			/// 通过域名连接服务器
			/// </summary>
			/// <param name="hostname"></param>
			/// <param name="port"></param>
			/// <returns>0成功,-1失败</returns>
			int connect_hostname(const char* hostname, int port);
			/// <summary>
			/// 通过地址连接服务器(SSL)
			/// </summary>
			/// <param name="address"></param>
			/// <param name="addrlen"></param>
			/// <returns>0成功,-1失败</returns>
			int connect_ssl(struct sockaddr* address, int addrlen);
			/// <summary>
			/// 通过域名连接服务器(SSL)
			/// </summary>
			/// <param name="hostname"></param>
			/// <param name="port"></param>
			/// <returns>0成功,-1失败</returns>
			int connect_hostname_ssl(const char* hostname, int port);

			/// <summary>
			/// 可以读数据回调
			/// </summary>
			/// <param name="bev"></param>
			virtual void input(const std::shared_ptr<EventBuffer> bev) = 0;
			/// <summary>
			/// 指定数据已经完成发送到底层socket回调
			/// </summary>
			/// <param name="bev"></param>
			virtual void output(const std::shared_ptr<EventBuffer> bev) = 0;
			/// <summary>
			/// 连接成功
			/// </summary>
			/// <param name="bev"></param>
			virtual void success(const std::shared_ptr<EventBuffer> bev) = 0;
			/// <summary>
			/// 连接失败
			/// </summary>
			/// <param name="bev"></param>
			virtual void error(const std::shared_ptr<EventBuffer> bev) = 0;
		private:
			std::shared_ptr<EventBuffer> buffer_ = nullptr;
			std::shared_ptr<EventBufferSsl> bufferssl_ = nullptr;
			std::shared_ptr<EventBase> base_ = nullptr;
			evdns_base* dnsbase_ = nullptr;
			SSL_CTX* ctx_ = nullptr;
		};
	}
}
