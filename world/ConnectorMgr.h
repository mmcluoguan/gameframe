#pragma once
#include <list>
#include <mutex>
#include "shynet/Singleton.h"
#include "world/DbConnector.h"

namespace world {
	/// <summary>
	/// 服务器连接管理器
	/// </summary>
	class ConnectorMgr {
		friend class shynet::Singleton<ConnectorMgr>;
		ConnectorMgr();
	public:
		~ConnectorMgr();

		std::shared_ptr<DbConnector> db_connector();

		void add_dbctor(int connectid);
		void remove_dbctor(int connectid);
		std::shared_ptr<DbConnector> find_dbctor(int connectid);
	private:
		std::list<int> dbctor_ids_;
		std::mutex dbctor_ids_mtx_;
	};
}
