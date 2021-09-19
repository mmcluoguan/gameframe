#ifndef FRMPUB_CONNECTOR_H
#define FRMPUB_CONNECTOR_H

#include "frmpub/FilterData.h"
#include "frmpub/PingTimer.h"
#include "shynet/net/ConnectEvent.h"

namespace frmpub {
	/*
	* 服务器连接器
	*/
	class Connector : public net::ConnectEvent, public FilterData {
	public:
		/// <summary>
		/// 服务器连接器
		/// </summary>
		/// <param name="connect_addr">需要连接的服务器地址</param>
		/// <param name="enable_ssl">是否启用ssl</param>
		/// <param name="enable_ping">是否发送心跳</param>
		/// <param name="heartSecond">心跳时间间隔(s)</param>
		/// <param name="pt">协议类型</param>
		/// <param name="pd">数据封包类型</param>
		Connector(std::shared_ptr<net::IPAddress> connect_addr,
			std::string name = "Connector",
			bool enable_ssl = false,
			bool enable_ping = false,
			ssize_t heartSecond = 5,
			protocol::FilterProces::ProtoType pt = protocol::FilterProces::ProtoType::SHY,
			FilterData::ProtoData pd = FilterData::ProtoData::PROTOBUF);
		~Connector();

		/// <summary>
		/// 与服务器连接断开回调
		/// </summary>
		/// <param name="active">断开原因</param>
		void close(net::ConnectEvent::CloseType active) override;
		/// <summary>
		/// 心跳超时回调
		/// </summary>
		void timerout() override;
		/// <summary>
		/// 连接成功
		/// </summary>
		void success() override;
		/// <summary>
		/// 可以读数据回调
		/// </summary>
		/// <returns>
		/// 返回0成功
		/// 返回-1服务器将关闭底层socket，并触发close(true)
		/// 返回-2服务器将关闭底层socket，并触发close(false)
		/// </returns>
		int input() override;
		/*
		* 消息数据封包处理
		*/
		int message_handle(char* original_data, size_t datalen) override;

		/// <summary>
		/// 与服务器连接断开回调
		/// </summary>
		/// <param name="active">断开原因</param>
		net::ConnectEvent::CloseType active() const;

		/*
		* 获取连接器名称
		*/
		std::string name() const {
			return name_;
		}
	private:
		net::ConnectEvent::CloseType active_ = net::ConnectEvent::CloseType::CLIENT_CLOSE;
		std::weak_ptr<PingTimer> ping_timer_;
		bool enable_ping_ = false;
		ssize_t heartSecond_ = 5;
		std::string name_{ "Connector" };
	};
}

#endif
