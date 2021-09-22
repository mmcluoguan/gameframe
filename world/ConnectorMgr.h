#ifndef WORLD_CONNECTORMGR_H
#define WORLD_CONNECTORMGR_H

#include "world/DbConnector.h"
#include "shynet/utils/Singleton.h"
#include <list>

namespace world {
	/// <summary>
	/// 服务器连接管理器
	/// </summary>
	class ConnectorMgr {
		friend class shynet::utils::Singleton<ConnectorMgr>;
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

#endif
