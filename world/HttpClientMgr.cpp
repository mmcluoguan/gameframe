#include "world/HttpClientMgr.h"
#include "frmpub/LuaCallBackTask.h"
#include "shynet/lua/LuaEngine.h"

namespace world {
	HttpClientMgr::HttpClientMgr() {
	}

	HttpClientMgr::~HttpClientMgr() {
	}

	void HttpClientMgr::add(int k, std::shared_ptr<HttpClient> v) {
		std::lock_guard<std::mutex> lock(clis_mutex_);
		clis_.insert({ k,v });

		//通知lua的onAccept函数
		shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
			std::make_shared<frmpub::OnAcceptTask<HttpClient>>(v));
	}

	bool HttpClientMgr::remove(int k) {
		std::lock_guard<std::mutex> lock(clis_mutex_);
		if (clis_.erase(k) > 0) {

			//通知lua的onClose函数
			shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
				std::make_shared<frmpub::OnCloseTask>(k));

			return true;
		}
		return false;
	}

	std::shared_ptr<HttpClient> HttpClientMgr::find(int k) {
		std::lock_guard<std::mutex> lock(clis_mutex_);
		return clis_[k];
	}

	std::unordered_map<int, std::shared_ptr<HttpClient>> HttpClientMgr::clis() const {
		std::lock_guard<std::mutex> lock(clis_mutex_);
		return clis_;
	}
	const net::IPAddress& HttpClientMgr::listen_addr() const {
		return listen_addr_;
	}
	void HttpClientMgr::set_listen_addr(const net::IPAddress& addr) {
		listen_addr_ = addr;
	}
}
