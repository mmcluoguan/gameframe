#pragma once
#include "frmpub/Connector.h"

namespace gate {
	/// <summary>
	/// 游戏服连接器
	/// </summary>
	class GameConnector : public frmpub::Connector, public std::enable_shared_from_this<GameConnector> {
	public:
		GameConnector(std::shared_ptr<net::IPAddress> connect_addr);
		~GameConnector();

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

		/*
		* 获取或设置连接id
		*/
		void game_conncet_id(int v);
		int game_conncet_id() const;
	private:
		/// <summary>
		/// 服务器通用错误信息
		/// </summary>
		int errcode(std::shared_ptr<protocc::CommonObject> data,
			std::shared_ptr<std::stack<FilterData::Envelope>> enves);

		/// <summary>
		/// 转发消息到client
		/// </summary>
		int forward_game_client_c(std::shared_ptr<protocc::CommonObject> data,
			std::shared_ptr<std::stack<FilterData::Envelope>> enves);
	private:
		int game_connect_id_;
	};
}
