#include "shynet/net/ConnectReactorMgr.h"
#include "shynet/pool/ThreadPool.h"
#include "shynet/thread/ConnectThread.h"
#include "shynet/utils/Logger.h"

namespace shynet {
	namespace net {

		ConnectReactorMgr::ConnectReactorMgr() {
		}

		ConnectReactorMgr::~ConnectReactorMgr() {
		}

		int ConnectReactorMgr::add(std::shared_ptr<ConnectEvent> v) {
			static int connectid = 0;
			{
				std::lock_guard<std::mutex> lock(cnt_mutex_);
				connectid++;
				cnts_.insert({ connectid,v });
				v->connectid(connectid);
			}
			notify(&connectid, sizeof(connectid));
			return connectid;
		}

		bool ConnectReactorMgr::remove(int k) {
			std::lock_guard<std::mutex> lock(cnt_mutex_);
			return cnts_.erase(k) > 0 ? true : false;
		}

		std::shared_ptr<ConnectEvent> ConnectReactorMgr::find(int k) {
			std::lock_guard<std::mutex> lock(cnt_mutex_);
			return cnts_[k];
		}

		std::shared_ptr<ConnectEvent> ConnectReactorMgr::find(const std::string ip, unsigned short port) {
			std::lock_guard<std::mutex> lock(cnt_mutex_);
			for (auto&& [key, value] : cnts_) {
				if (value != nullptr) {
					if (value->connect_addr()->ip() == ip &&
						value->connect_addr()->port() == port) {
						return value;
					}
				}
			}
			return nullptr;
		}

		void ConnectReactorMgr::notify(const void* data, size_t len) {
			std::shared_ptr<thread::ConnectThread> cnt = utils::Singleton<pool::ThreadPool>::get_instance().connectTh().lock();
			if (cnt != nullptr) {
				cnt->notify(data, len);
			}
			else
				LOG_WARN << "没有可用的 ConnectThread";
		}

	}
}
