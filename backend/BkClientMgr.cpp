#include "backend/BkClientMgr.h"

namespace backend {
	BkClientMgr::BkClientMgr() {
		GOBJ++;
		LOG_TRACE << "BkClientMgr:" << GOBJ;
	}

	BkClientMgr::~BkClientMgr() {
		GOBJ--;
		LOG_TRACE << "~BkClientMgr:" << GOBJ;
	}

	void BkClientMgr::add(int k, std::shared_ptr<BkClient> v) {
		std::lock_guard<std::mutex> lock(clis_mutex_);
		clis_.insert({ k,v });
	}

	bool BkClientMgr::remove(int k) {
		std::lock_guard<std::mutex> lock(clis_mutex_);
		return clis_.erase(k) > 0 ? true : false;
	}

	std::shared_ptr<BkClient> BkClientMgr::find(int k) {
		std::lock_guard<std::mutex> lock(clis_mutex_);
		return clis_[k];
	}

	std::shared_ptr<BkClient> BkClientMgr::find(int64_t phone) {
		std::lock_guard<std::mutex> lock(clis_mutex_);
		for (auto& it : clis_) {
			if (it.second->phone() == phone) {
				return it.second;
			}
		}
		return nullptr;
	}

	std::unordered_map<int, std::shared_ptr<BkClient>> BkClientMgr::clis() const {
		std::lock_guard<std::mutex> lock(clis_mutex_);
		return clis_;
	}

	const net::IPAddress& BkClientMgr::listen_addr() const {
		return listen_addr_;
	}
	void BkClientMgr::listen_addr(const net::IPAddress& addr) {
		listen_addr_ = addr;
	}
}
