#include "dbvisit/DataTimer.h"
#include <vector>
#include "shynet/net/TimerReactorMgr.h"
#include "shynet/utils/StringOp.h"
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
<<<<<<< HEAD
			std::vector<std::string> temp = shynet::utils::StringOp::split(cachekey_, "_");
=======
			std::vector<std::string> temp = shynet::utils::StringOp::spilt(cachekey_, "_");
>>>>>>> 97f5d8ccc1392d6c54dfc663b535a803fe2f1f9e
			shynet::utils::Singleton<Datahelp>::instance().updata_db(temp[0], temp[1], fields_);
			shynet::utils::Singleton<net::TimerReactorMgr>::instance().remove(timerid());
			shynet::utils::Singleton<DataTimerMgr>::instance().remove(cachekey_);
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
