#include "gate/GateClientMgr.h"
#include "shynet/lua/LuaEngine.h"
#include "frmpub/LuaCallBackTask.h"

namespace gate {
	GateClientMgr::GateClientMgr() {
	}

	GateClientMgr::~GateClientMgr() {
	}

	void GateClientMgr::add(int k, std::shared_ptr<GateClient> v) {
		std::lock_guard<std::mutex> lock(clis_mutex_);
		clis_.insert({ k,v });

		//通知lua的onAccept函数
		shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
			std::make_shared<frmpub::OnAcceptTask<GateClient>>(v));
	}

	bool GateClientMgr::remove(int k) {
		std::lock_guard<std::mutex> lock(clis_mutex_);
		if (clis_.erase(k) > 0) {

			//通知lua的onClose函数
			shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
				std::make_shared<frmpub::OnCloseTask>(k));

			return true;
		}
		return false;
	}

	std::shared_ptr<GateClient> GateClientMgr::find(int k) {
		std::lock_guard<std::mutex> lock(clis_mutex_);
		return clis_[k];
	}

	std::shared_ptr<GateClient> GateClientMgr::find(const std::string& key) {
		std::lock_guard<std::mutex> lock(clis_mutex_);
		for (auto& iter : clis_) {
			if (iter.second->set_accountid() == key ||
				iter.second->set_platform_key() == key) {
				return iter.second;
			}
		}
		return nullptr;
	}

	std::unordered_map<int, std::shared_ptr<GateClient>> GateClientMgr::clis() const {
		std::lock_guard<std::mutex> lock(clis_mutex_);
		return clis_;
	}

	const net::IPAddress& GateClientMgr::listen_addr() const {
		return listen_addr_;
	}
	void GateClientMgr::set_listen_addr(const net::IPAddress& addr) {
		listen_addr_ = addr;
	}
}
