#include "dbvisit/DataTimer.h"
#include "dbvisit/DataTimerMgr.h"
#include "dbvisit/Datahelp.h"
#include "shynet/net/TimerReactorMgr.h"
#include "shynet/utils/StringOp.h"
#include <vector>

namespace dbvisit {
DataTimer::DataTimer(const std::string& cachekey, const struct timeval val)
    : net::TimerEvent(val, EV_TIMEOUT)
{
    cachekey_ = cachekey;
}

DataTimer::~DataTimer()
{
}

void DataTimer::timeout()
{
    try {
        std::vector<std::string> temp = shynet::utils::StringOp::split(cachekey_, "_");
        shynet::utils::Singleton<Datahelp>::instance().updata_db(temp[0], temp[1], fields_);
        shynet::utils::Singleton<net::TimerReactorMgr>::instance().remove(timerid());
        shynet::utils::Singleton<DataTimerMgr>::instance().remove(cachekey_);
        LOG_DEBUG << "更新数据到 tablename:" << temp[0] << " key:" << temp[1];
    } catch (std::exception& err) {
        THROW_EXCEPTION(err.what());
    }
}

void DataTimer::modify_cache_fields(const std::unordered_map<std::string, std::string>& data)
{
    for (auto&& [key, value] : data) {
        fields_[key] = value;
    }
}
}
