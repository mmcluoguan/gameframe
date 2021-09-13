#include "world/WorldClientMgr.h"
#include "shynet/lua/LuaEngine.h"
#include "frmpub/LuaCallBackTask.h"

namespace world {
	WorldClientMgr::WorldClientMgr() {
	}

	WorldClientMgr::~WorldClientMgr() {
	}

	void WorldClientMgr::add(int k, std::shared_ptr<WorldClient> v) {
		std::lock_guard<std::mutex> lock(clis_mutex_);
		clis_.insert({ k,v });

		//通知lua的onAccept函数
		shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
			std::make_shared<frmpub::OnAcceptTask<WorldClient>>(v));
	}

	bool WorldClientMgr::remove(int k) {
		std::lock_guard<std::mutex> lock(clis_mutex_);
		if (clis_.erase(k) > 0) {

			//通知lua的onClose函数
			shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
				std::make_shared<frmpub::OnCloseTask>(k));

			return true;
		}
		return false;
	}

	std::shared_ptr<WorldClient> WorldClientMgr::find(int k) {
		std::lock_guard<std::mutex> lock(clis_mutex_);
		return clis_[k];
	}

	std::unordered_map<int, std::shared_ptr<WorldClient>> WorldClientMgr::clis() const {
		std::lock_guard<std::mutex> lock(clis_mutex_);
		return clis_;
	}
	const net::IPAddress& WorldClientMgr::listen_addr() const {
		return listen_addr_;
	}
	void WorldClientMgr::listen_addr(const net::IPAddress& addr) {
		listen_addr_ = addr;
	}

	std::shared_ptr<WorldClient> WorldClientMgr::select_game() {
		std::lock_guard<std::mutex> lock(clis_mutex_);
		std::shared_ptr<WorldClient> game;
		int min = -1;
		for (auto& it : clis_) {
			if (it.second->sif().st() == protocc::ServerType::GAME) {
				if (min == -1) {
					min = it.second->connect_num();
					game = it.second;
				}
				else if (it.second->connect_num() < min) {
					min = it.second->connect_num();
					game = it.second;
				}
			}
		}
		if (game != nullptr) {

			game->connect_num(game->connect_num() + 1);
		}
		return game;
	}
}
