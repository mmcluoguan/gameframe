#include "dbvisit/DbClientMgr.h"
#include "shynet/lua/LuaEngine.h"
#include "frmpub/LuaCallBackTask.h"

namespace dbvisit {
	DbClientMgr::DbClientMgr() {
	}

	DbClientMgr::~DbClientMgr() {
	}

	void DbClientMgr::add(int k, std::shared_ptr<DbClient> v) {
		std::lock_guard<std::mutex> lock(clis_mutex_);
		clis_.insert({ k,v });

		//通知lua的onAccept函数
		shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
			std::make_shared<frmpub::OnAcceptTask<DbClient>>(v));
	}

	bool DbClientMgr::remove(int k) {
		std::lock_guard<std::mutex> lock(clis_mutex_);
		if (clis_.erase(k) > 0) {

			//通知lua的onClose函数
			shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
				std::make_shared<frmpub::OnCloseTask>(k));

			return true;
		}
		return false;
	}

	std::shared_ptr<DbClient> DbClientMgr::find(int k) {
		std::lock_guard<std::mutex> lock(clis_mutex_);
		return clis_[k];
	}

	std::unordered_map<int, std::shared_ptr<DbClient>> DbClientMgr::clis() const {
		std::lock_guard<std::mutex> lock(clis_mutex_);
		return clis_;
	}

	const net::IPAddress& DbClientMgr::listen_addr() const {
		return listen_addr_;
	}
	void DbClientMgr::listen_addr(const net::IPAddress& addr) {
		listen_addr_ = addr;
	}
	std::shared_ptr<DbClient> DbClientMgr::find(std::string sid) {
		std::lock_guard<std::mutex> lock(clis_mutex_);
		for (auto& iter : clis_) {
			if (std::to_string(iter.second->sif().sid()) == sid) {
				return iter.second;
			}
		}
		return nullptr;
	}
}
