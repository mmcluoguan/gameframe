#pragma once
#include <mutex>
#include <unordered_map>
#include "shynet/Singleton.h"
#include "gate/DbConnector.h"
#include "gate/LoginConnector.h"
#include "gate/WorldConnector.h"
#include "gate/GameConnector.h"

namespace gate {
	/// <summary>
	/// 服务器连接管理器
	/// </summary>
	class ConnectorMgr {
		friend class shynet::Singleton<ConnectorMgr>;
		ConnectorMgr();
	public:
		/// <summary>
		/// 服务器连接信息
		/// </summary>
		struct ConnectData {
			int connect_id = 0;
			protocc::ServerInfo sif;
			int connect_num = 0; //服务器接收玩家数量
		};
	public:
		~ConnectorMgr();

		/*
		* 服务id转换连接id
		*/
		int sid_conv_connect_id(int sid);

		/// <summary>
		/// 添加新的服务器连接
		/// </summary>
		/// <param name="connect_id">新的服务器连接id</param>
		/// <param name="sif">新的服务器信息</param>
		void add_new_connect_data(int connect_id, protocc::ServerInfo sif);
		/// <summary>
		/// 删除服务器连接信息
		/// </summary>
		/// <param name="connect_id">服务器连接id</param>
		/// <returns></returns>
		bool remove(int connect_id);
		/// <summary>
		/// 通过服务id查找连接信息
		/// </summary>
		/// <param name="connect_id">服务器连接id</param>
		/// <returns></returns>
		ConnectData find_connect_data(int connect_id);

		/*
		* 选择登录服务器，如果指定login_id找不到,默认选择connect_num值最小的登录服务器
		*/
		std::shared_ptr<LoginConnector> select_login(int login_id);
		/*
		* 选择游戏服务器，如果指定game_id找不到,默认选择connect_num值最小的游戏服务器
		*/
		std::shared_ptr<GameConnector> select_game(int game_id);


		/// <summary>
		/// 减少指定连接的服务中接收玩家数量1
		/// </summary>
		/// <param name="connect_id">服务器连接id</param>
		/// <returns></returns>
		bool reduce_count(int connect_id);

		std::shared_ptr<WorldConnector> world_connector() const;
		void world_id(int id);

		std::shared_ptr<DbConnector> db_connector() const;
		void db_id(int id);

		/// <summary>
		/// 获取登录服务器连接
		/// </summary>
		/// <param name="login_id">服务器连接id</param>
		/// <returns></returns>
		std::shared_ptr<LoginConnector> login_connector(int login_id) const;

		/// <summary>
		/// 获取游戏服务器连接
		/// </summary>
		/// <param name="game_id">服务器连接id</param>
		/// <returns></returns>
		std::shared_ptr<GameConnector> game_connector(int game_id) const;

		/// <summary>
		/// 服务器连接信息列表
		/// key为conncet_id
		/// </summary>
		/// <returns></returns>
		std::unordered_map<int, ConnectData> connect_datas() const;

	private:
		int world_id_;
		int db_id_;
		mutable std::mutex connect_data_mutex_;
		std::unordered_map<int, ConnectData> connect_datas_;
	};
}
