#pragma once
#include "shynet/Singleton.h"
#include "game/DbConnector.h"
#include "game/WorldConnector.h"

namespace game {
	/// <summary>
	/// 服务器连接管理器
	/// </summary>
	class ConnectorMgr {
		friend class shynet::Singleton<ConnectorMgr>;
		ConnectorMgr();
	public:
		~ConnectorMgr();

		std::shared_ptr<DbConnector> db_connector() const;
		std::shared_ptr<WorldConnector> world_connector() const;

		int dbctor_id() const;
		void dbctor_id(int v);
		int world_id() const;
		void world_id(int v);
	private:
		int dbctor_id_;
		int world_id_;
	};
}
