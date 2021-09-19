#pragma once
#include "frmpub/Connector.h"

namespace gate {
	/// <summary>
	/// 登录服连接器
	/// </summary>
	class LoginConnector : public frmpub::Connector, public std::enable_shared_from_this<LoginConnector> {
	public:
		LoginConnector(std::shared_ptr<net::IPAddress> connect_addr);
		~LoginConnector();

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
		void login_conncet_id(int v);
		int set_login_conncet_id() const;
	private:
		/// <summary>
		/// 服务器通用错误信息
		/// </summary>
		int errcode(std::shared_ptr<protocc::CommonObject> data,
			std::shared_ptr<std::stack<FilterData::Envelope>> enves);

		/// <summary>
		/// 转发消息到client
		/// </summary>
		int forward_login_client_c(std::shared_ptr<protocc::CommonObject> data,
			std::shared_ptr<std::stack<FilterData::Envelope>> enves);

		/// <summary>
		/// 跨服顶号处理
		/// </summary>
		int repeatlogin_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
			std::shared_ptr<std::stack<FilterData::Envelope>> enves);
	private:
		int login_connect_id_;
	};
}