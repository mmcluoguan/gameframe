#ifndef LOGIN_LOGINCLIENT_H
#define LOGIN_LOGINCLIENT_H

#include "frmpub/Client.h"
#include "shynet/net/AcceptNewFd.h"

namespace login {
	/// <summary>
	/// 登录服客户端
	/// </summary>
	class LoginClient : public frmpub::Client, public std::enable_shared_from_this<LoginClient> {
	public:
		LoginClient(std::shared_ptr<net::IPAddress> remote_addr,
			std::shared_ptr<net::IPAddress> listen_addr,
			std::shared_ptr<events::EventBuffer> iobuf);
		~LoginClient();

		/// <summary>
		/// 处理网络消息
		/// </summary>
		/// <param name="obj">数据包</param>
		/// <param name="enves">数据包转发路由</param>
		/// <returns></returns>
		int input_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<FilterData::Envelope>> enves) override;

		/// <summary>
		/// 连接断开
		/// </summary>
		/// <param
		/// name="active">true服务器主动断开,false客户端主动断开</param>
		void close(bool active) override;
	private:
	private:
		/// <summary>
		/// 服务器通用错误信息
		/// </summary>
		int errcode(std::shared_ptr<protocc::CommonObject> data,
			std::shared_ptr<std::stack<FilterData::Envelope>> enves);
		/*
		* 网关服注册
		*/
		int register_gate_login_c(std::shared_ptr<protocc::CommonObject> data,
			std::shared_ptr<std::stack<FilterData::Envelope>> enves);

		/*
		* 登陆
		*/
		int login_client_gate_c(std::shared_ptr<protocc::CommonObject> data,
			std::shared_ptr<std::stack<FilterData::Envelope>> enves);

		/*
		* 转发db服处理
		*/
		int forward_client_gate_c(std::shared_ptr<protocc::CommonObject> data,
			std::shared_ptr<std::stack<FilterData::Envelope>> enves);

		/*
		* 玩家下线
		*/
		int clioffline_gate_all_c(std::shared_ptr<protocc::CommonObject> data,
			std::shared_ptr<std::stack<FilterData::Envelope>> enves);

		/*
		* 玩家创建角色成功
		*/
		int createrole_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
			std::shared_ptr<std::stack<FilterData::Envelope>> enves);
	};
}

#endif
