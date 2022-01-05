#include "dbvisit/datatimer.h"
#include "dbvisit/datahelp.h"
#include "dbvisit/datatimermgr.h"
#include "shynet/net/timerreactormgr.h"
#include "shynet/utils/elapsed.h"
#include "shynet/utils/stringop.h"
#include <vector>

namespace dbvisit {
DataTimer::DataTimer(const std::string& cachekey, const struct timeval val)
    : net::TimerEvent(val, EV_TIMEOUT)
{
    cachekey_ = cachekey;
}

void DataTimer::timeout()
{
    auto cb = [&]() {
        try {
            std::vector<std::string> temp = shynet::utils::stringop::split(cachekey_, "_");
            LOG_DEBUG << "更新数据到 tablename:" << temp[0] << " key:" << temp[1];

            shynet::utils::Singleton<Datahelp>::instance().updata_db(temp[0], temp[1], fields_);
            shynet::utils::Singleton<net::TimerReactorMgr>::instance().remove(timerid());
            shynet::utils::Singleton<DataTimerMgr>::instance().remove(cachekey_);
        } catch (std::exception& err) {
            THROW_EXCEPTION(err.what());
        }
    };

#ifdef USE_DEBUG
    shynet::utils::elapsed("工作线程计时单任务执行 DataTimer");
    return cb();
#elif
    return cb();
#endif
}

void DataTimer::modify_cache_fields(const std::unordered_map<std::string, std::string>& data)
{
    for (auto&& [key, value] : data) {
        fields_[key] = value;
    }
}
}
