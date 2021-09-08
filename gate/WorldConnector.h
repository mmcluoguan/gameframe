#pragma once
#include <unordered_map>
#include <functional>
#include "frmpub/Connector.h"

namespace gate {
	/// <summary>
	/// 世界服连接器
	/// </summary>
	class WorldConnector : public frmpub::Connector, public std::enable_shared_from_this<WorldConnector> {
	public:
		WorldConnector(std::shared_ptr<net::IPAddress> connect_addr);
		~WorldConnector();

		/// <summary>
		/// 连接完成
		/// </summary>
		void complete() override;

		/// <summary>
		/// 处理网络消息
		/// </summary>
		/// <param name="obj">数据包</param>
		/// <param name="enves">数据包转发路由</param>
		/// <returns></returns>
		int input_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<FilterData::Envelope>> enves) override;

		/// <summary>
		/// 与服务器连接断开回调
		/// </summary>
		/// <param name="active">断开原因</param>
		void close(net::ConnectEvent::CloseType active) override;
	private:
		/// <summary>
		/// 服务器通用错误信息
		/// </summary>
		int errcode(std::shared_ptr<protocc::CommonObject> data,
			std::shared_ptr<std::stack<FilterData::Envelope>> enves);
		/*
		* 网关服向世界服注册结果
		*/
		int register_gate_world_s(std::shared_ptr<protocc::CommonObject> data,
			std::shared_ptr<std::stack<FilterData::Envelope>> enves);

		/*
		* 其他服务器上线
		*/
		int seronline_world_gate_g(std::shared_ptr<protocc::CommonObject> data,
			std::shared_ptr<std::stack<FilterData::Envelope>> enves);

		/// <summary>
		/// 转发消息到client
		/// </summary>
		int forward_world_client_c(std::shared_ptr<protocc::CommonObject> data,
			std::shared_ptr<std::stack<FilterData::Envelope>> enves);
	private:
	};
}
