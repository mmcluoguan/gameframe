#pragma once
#include "shynet/events/EventBuffer.h"
#include "frmpub/Client.h"

namespace world {
	/// <summary>
	/// 后台客户端
	/// </summary>
	class HttpClient : public frmpub::Client, public std::enable_shared_from_this<HttpClient> {
	public:
		HttpClient(std::shared_ptr<net::IPAddress> remote_addr,
			std::shared_ptr<net::IPAddress> listen_addr,
			std::shared_ptr<events::EventBuffer> iobuf);
		~HttpClient();

		/// <summary>
		/// 处理网络消息
		/// </summary>
		/// <param name="obj">数据包</param>
		/// <param name="enves">数据包转发路由</param>
		/// <returns></returns>
		int input_handle(std::shared_ptr<rapidjson::Document> doc, std::shared_ptr<std::stack<FilterData::Envelope>> enves) override;

		/// <summary>
		/// 连接断开
		/// </summary>
		/// <param name="active">true服务器主动断开,false客户端主动断开</param>
		void close(bool active) override;
	private:
	};
}
