#pragma once
#include <sw/redis++/redis++.h>
namespace redis = sw::redis;
#include <unordered_map>
#include <functional>
#include "shynet/events/EventBuffer.h"
#include "frmpub/Client.h"

namespace dbvisit {
	/// <summary>
	/// db客户端连接
	/// </summary>
	class DbClient : public frmpub::Client, public std::enable_shared_from_this<DbClient> {
	public:
		/// <summary>
		/// 客户端连接
		/// </summary>
		/// <param name="remote_addr">连接端地址</param>
		/// <param name="listen_addr">服务器监听地址</param>
		/// <param name="iobuf">连接端io读写缓冲区</param>
		DbClient(std::shared_ptr<net::IPAddress> remote_addr,
			std::shared_ptr<net::IPAddress> listen_addr,
			std::shared_ptr<events::EventBuffer> iobuf);
		~DbClient();

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
		/// <param name="active">true服务器主动断开,false客户端主动断开</param>
		void close(bool active) override;
	private:
		std::chrono::seconds oneday_ = std::chrono::seconds(24 * 60 * 60);
	private:
		/*
		* 验证服务是否已经注册
		*/
		bool verify_register(const protocc::ServerInfo& sif);

		/// <summary>
		/// 服务器通用错误信息
		/// </summary>
		int errcode(std::shared_ptr<protocc::CommonObject> data,
			std::shared_ptr<std::stack<FilterData::Envelope>> enves);

		/*
		* 游戏服注册
		*/
		int register_world_dbvisit_c(std::shared_ptr<protocc::CommonObject> data,
			std::shared_ptr<std::stack<FilterData::Envelope>> enves);
		/*
		* 登录服注册
		*/
		int register_login_dbvisit_c(std::shared_ptr<protocc::CommonObject> data,
			std::shared_ptr<std::stack<FilterData::Envelope>> enves);
		/*
		* 游戏服注册
		*/
		int register_game_dbvisit_c(std::shared_ptr<protocc::CommonObject> data,
			std::shared_ptr<std::stack<FilterData::Envelope>> enves);
		/*
		* 网关服注册
		*/
		int register_gate_dbvisit_c(std::shared_ptr<protocc::CommonObject> data,
			std::shared_ptr<std::stack<FilterData::Envelope>> enves);
		/*
		* 从dbvisit加载1条hash数据
		*/
		int loaddata_from_dbvisit_c(std::shared_ptr<protocc::CommonObject> data,
			std::shared_ptr<std::stack<FilterData::Envelope>> enves);
		/*
		* 从dbvisit加载多条hash数据
		*/
		int loaddata_more_from_dbvisit_c(std::shared_ptr<protocc::CommonObject> data,
			std::shared_ptr<std::stack<FilterData::Envelope>> enves);
		/*
		* 添加1条hash数据到dbvisit
		*/
		int insertdata_to_dbvisit_c(std::shared_ptr<protocc::CommonObject> data,
			std::shared_ptr<std::stack<FilterData::Envelope>> enves);
		/*
		* 更新1条hash数据到dbvisit
		*/
		int updata_to_dbvisit_c(std::shared_ptr<protocc::CommonObject> data,
			std::shared_ptr<std::stack<FilterData::Envelope>> enves);
		/*
		* 从dbvisit删除1条hash数据
		*/
		int deletedata_to_dbvisit_c(std::shared_ptr<protocc::CommonObject> data,
			std::shared_ptr<std::stack<FilterData::Envelope>> enves);
		/*
		* 登录
		*/
		int login_client_gate_c(std::shared_ptr<protocc::CommonObject> data,
			std::shared_ptr<std::stack<FilterData::Envelope>> enves);

		/*
		* 玩家下线
		*/
		int clioffline_gate_all_c(std::shared_ptr<protocc::CommonObject> data,
			std::shared_ptr<std::stack<FilterData::Envelope>> enves);

		/*
		* 断线重连
		*/
		int reconnect_client_gate_c(std::shared_ptr<protocc::CommonObject> data,
			std::shared_ptr<std::stack<FilterData::Envelope>> enves);
	};
}
