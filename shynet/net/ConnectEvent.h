#pragma once
#include "shynet/events/EventHandler.h"
#include "shynet/net/IPAddress.h"
#include "shynet/net/ConnectHeartbeat.h"
#include "shynet/protocol/FilterProces.h"
#include <event2/dns.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

namespace shynet {
	namespace net {
		/// <summary>
		/// 服务器连接事件
		/// </summary>
		class ConnectEvent : public protocol::FilterProces {
		public:
			/// <summary>
			/// 与服务器连接断开原因
			/// </summary>
			enum class CloseType {
				CLIENT_CLOSE, //客户端主动断开
				SERVER_CLOSE, //服务区主动断开
				CONNECT_FAIL, //连接服务器失败
				TIMEOUT_CLOSE,//与服务器心跳超时
			};

			/// <summary>
			/// 服务器连接事件
			/// </summary>
			/// <param name="connect_addr">需要连接的服务器地址</param>
			/// <param name="pt">协议类型</param>
			/// <param name="enable_ssl">是否启用ssl</param>
			/// <param name="enableHeart">是否启用验证服务器心跳</param>
			/// <param name="heartSecond">要求服务器维持心跳间隔秒数</param>
			ConnectEvent(std::shared_ptr<net::IPAddress> connect_addr,
				FilterProces::ProtoType pt,
				bool enable_ssl = false,
				bool enableHeart = true, ssize_t heartSecond = 5);

			/// <summary>
			/// 服务器连接事件
			/// </summary>
			/// <param name="hostname">服务器主机名</param>
			/// <param name="port">端口</param>
			/// <param name="pt">协议类型</param>
			/// <param name="enable_ssl">是否启用ssl</param>
			/// <param name="enableHeart">是否启用验证服务器心跳</param>
			/// <param name="heartSecond">要求服务器维持心跳间隔秒数</param>
			ConnectEvent(const char* hostname, short port,
				FilterProces::ProtoType pt,
				bool enable_ssl = false,
				bool enableHeart = true, ssize_t heartSecond = 5);
			~ConnectEvent();

			/*
			* 获取设置连接器id
			*/
			int connectid() const;
			void connectid(int id);

			/*
			* 连接的服务器地址
			*/
			std::shared_ptr<net::IPAddress> connect_addr() const;
			/*
			* 是否启用ssl
			*/
			bool enable_ssl() const;
			SSL_CTX* ctx() const;

			/// <summary>
			/// 连接成功
			/// </summary>
			virtual void success();
			/// <summary>
			/// 可以读数据回调
			/// </summary>
			/// <returns>
			/// 返回0成功
			/// 返回-1服务器将关闭底层socket，并触发close(true)
			/// 返回-2服务器将关闭底层socket，并触发close(false)
			/// </returns>
			virtual int input();
			/// <summary>
			/// 指定数据已经完成发送到底层socket回调
			/// </summary>
			/// <returns>
			/// 返回0成功
			/// 返回-1服务器将关闭底层socket，并触发close(true)
			/// 返回-2服务器将关闭底层socket，并触发close(false)
			/// </returns>
			virtual int output() { return 0; };
			/// <summary>
			/// 与服务器连接断开回调
			/// </summary>
			/// <param name="active">断开原因</param>
			virtual void close(CloseType active) = 0;
			/// <summary>
			/// 心跳超时回调
			/// </summary>
			virtual void timerout() = 0;

			/*
			* 是否启用验证服务器心跳
			*/
			bool enable_heart() const;
			/*
			* 要求服务器维持心跳间隔秒数
			*/
			ssize_t heart_second() const;

			int fd() const {
				return iobuf()->fd();
			}

			/*
			 * 心跳计时器
			*/
			std::weak_ptr<ConnectHeartbeat> heart() const;
			void set_heart(std::weak_ptr<ConnectHeartbeat> ht);

			bool enable_dns() const;
			short dnsport() const;
			void dnsbase(evdns_base* base);
			evdns_base* set_dnsbase() const;
			std::string hostname() const;

		private:
			std::shared_ptr<net::IPAddress> connect_addr_ = nullptr;
			bool enable_ssl_ = false;
			SSL_CTX* ctx_ = nullptr;
			int conectid_ = -1;
			bool enable_heart_ = true;
			ssize_t heart_second_ = 5;
			std::weak_ptr<ConnectHeartbeat> heart_;
			evdns_base* dnsbase_ = nullptr;
			short dnsport_ = 0;
			bool enable_dns_ = false;
			std::string hostname_;
		};

	}
}
