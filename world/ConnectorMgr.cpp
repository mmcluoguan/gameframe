#include "world/ConnectorMgr.h"
#include "shynet/net/ConnectReactorMgr.h"

namespace world {
	ConnectorMgr::ConnectorMgr() {
	}
	ConnectorMgr::~ConnectorMgr() {
	}
	std::shared_ptr<DbConnector> ConnectorMgr::db_connector() {
		if (dbctor_ids_.begin() != dbctor_ids_.end()) {
			return find_dbctor(*dbctor_ids_.begin());
		}
		return nullptr;
	}
	void ConnectorMgr::add_dbctor(int connectid) {
		std::lock_guard<std::mutex> lock(dbctor_ids_mtx_);
		dbctor_ids_.push_back(connectid);
	}
	void ConnectorMgr::remove_dbctor(int connectid) {
		std::lock_guard<std::mutex> lock(dbctor_ids_mtx_);
		dbctor_ids_.remove(connectid);
	}
	std::shared_ptr<DbConnector> ConnectorMgr::find_dbctor(int connectid) {
		return std::dynamic_pointer_cast<DbConnector>(
			shynet::utils::Singleton<net::ConnectReactorMgr>::instance().find(connectid));
	}
}
