#pragma once
#include <openssl/err.h>
#include <openssl/ssl.h>
#include "shynet/net/IPAddress.h"
#include "shynet/net/AcceptNewFd.h"

namespace shynet
{
	namespace net
	{
		/// <summary>
		/// 服务器监听事件
		/// </summary>
		class ListenEvent : public events::EventHandler
		{
		public:
			/// <summary>
			/// 服务器监听事件
			/// </summary>
			/// <param name="listen_addr">服务器监听的地址</param>
			/// <param name="enable_ssl">是否启用ssl</param>
			ListenEvent(std::shared_ptr<net::IPAddress> listen_addr, bool enable_ssl = false);
			~ListenEvent();

			/*
			* 获取设置服务器id
			*/
			int serverid() const;
			void set_serverid(int id);

			std::shared_ptr<net::IPAddress> listenaddr() const;
			int listenfd() const;
			bool enable_ssl() const;
			SSL_CTX* ctx() const;

			/*
			* 操作系统底层socket准备完成回调
			*/
			void input(int listenfd) override;
			/*
			* 关闭服务器
			*/
			bool stop() const;

			/// <summary>
			/// 创建新连接处理实例
			/// </summary>
			/// <param name="remoteAddr">客户端连接地址</param>
			/// <param name="iobuf">io读写缓冲区</param>
			/// <returns>新连接处理实例</returns>
			virtual std::weak_ptr<net::AcceptNewFd> accept_newfd(
				std::shared_ptr<net::IPAddress> remoteAddr,
				std::shared_ptr<events::EventBuffer> iobuf) = 0;
		private:
			ListenEvent* self = nullptr;
			int listenfd_ = 0;
			int serverid_ = -1;
			std::shared_ptr<net::IPAddress> listen_addr_ = nullptr;
			std::shared_ptr<events::EventBase> base_ = nullptr;
			bool enable_ssl_ = false;
			SSL_CTX* ctx_ = nullptr;
		};
	}
}
