#include "game/ConnectorMgr.h"
#include "shynet/net/ConnectReactorMgr.h"

namespace game {
	ConnectorMgr::ConnectorMgr() {
	}
	ConnectorMgr::~ConnectorMgr() {
	}
	std::shared_ptr<DbConnector> ConnectorMgr::db_connector() const {
		return std::dynamic_pointer_cast<DbConnector>(
			shynet::Singleton<net::ConnectReactorMgr>::instance().find(dbctor_id_));
	}
	std::shared_ptr<WorldConnector> ConnectorMgr::world_connector() const {
		return std::dynamic_pointer_cast<WorldConnector>(
			shynet::Singleton<net::ConnectReactorMgr>::instance().find(world_id_));
	}
	int ConnectorMgr::dbctor_id() const {
		return dbctor_id_;
	}
	void ConnectorMgr::dbctor_id(int v) {
		dbctor_id_ = v;
	}
	int ConnectorMgr::world_id() const {
		return world_id_;
	}
	void ConnectorMgr::world_id(int v) {
		world_id_ = v;
	}
}
