#include "animal/AmClientMgr.h"

namespace animal {
	AmClientMgr::AmClientMgr() {
		GOBJ++;
		LOG_TRACE << "AmClientMgr:" << GOBJ;
	}

	AmClientMgr::~AmClientMgr() {
		GOBJ--;
		LOG_TRACE << "~AmClientMgr:" << GOBJ;
	}

	void AmClientMgr::add(int k, std::shared_ptr<AmClient> v) {
		std::lock_guard<std::mutex> lock(clis_mutex_);
		clis_.insert({ k,v });
	}

	bool AmClientMgr::remove(int k) {
		std::lock_guard<std::mutex> lock(clis_mutex_);
		return clis_.erase(k) > 0 ? true : false;
	}

	std::shared_ptr<AmClient> AmClientMgr::find(int k) {
		std::lock_guard<std::mutex> lock(clis_mutex_);
		return clis_[k];
	}

	std::shared_ptr<AmClient> AmClientMgr::find(int64_t phone) {
		std::lock_guard<std::mutex> lock(clis_mutex_);
		for (auto& it : clis_) {
			if (it.second->phone() == phone) {
				return it.second;
			}
		}
		return nullptr;
	}

	std::unordered_map<int, std::shared_ptr<AmClient>> AmClientMgr::clis() const {
		std::lock_guard<std::mutex> lock(clis_mutex_);
		return clis_;
	}

	const net::IPAddress& AmClientMgr::listen_addr() const {
		return listen_addr_;
	}
	void AmClientMgr::listen_addr(const net::IPAddress& addr) {
		listen_addr_ = addr;
	}
}
