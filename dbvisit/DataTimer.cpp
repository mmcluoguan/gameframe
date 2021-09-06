#include "dbvisit/DataTimer.h"
#include "shynet/net/TimerReactorMgr.h"
#include "shynet/Singleton.h"
#include "dbvisit/Datahelp.h"
#include "dbvisit/DataTimerMgr.h"

namespace dbvisit {
	DataTimer::DataTimer(const std::string& cachekey, const struct timeval val) :
		net::TimerEvent(val, EV_TIMEOUT) {
		cachekey_ = cachekey;
	}

	DataTimer::~DataTimer() {
	}

	void DataTimer::timeout() {
		try {
			std::vector<std::string> temp = shynet::Utility::spilt(cachekey_, "_");
			shynet::Singleton<Datahelp>::instance().updata_db(temp[0], temp[1], fields_);
			shynet::Singleton<net::TimerReactorMgr>::instance().remove(timerid());
			shynet::Singleton<DataTimerMgr>::instance().remove(cachekey_);
			LOG_DEBUG << "更新数据到 tablename:" << temp[0] << " key:" << temp[1];
		}
		catch (std::exception& err) {
			LOG_WARN << err.what();
		}
	}

	void DataTimer::modify_cache_fields(const std::unordered_map<std::string, std::string>& data) {
		for (const auto& it : data) {
			fields_[it.first] = it.second;
		}
	}
}
