#include "dbvisit/datatimermgr.h"

namespace dbvisit {

void DataTimerMgr::add(std::string cachekey, int timerid)
{
    std::lock_guard<std::mutex> lock(cachekey_timerid_mutex_);
    cachekey_timerids_[cachekey] = timerid;
}

bool DataTimerMgr::remove(std::string cachekey)
{
    std::lock_guard<std::mutex> lock(cachekey_timerid_mutex_);
    return cachekey_timerids_.erase(cachekey) > 0 ? true : false;
}

int DataTimerMgr::find(std::string cachekey)
{
    std::lock_guard<std::mutex> lock(cachekey_timerid_mutex_);
    auto iter = cachekey_timerids_.find(cachekey);
    if (iter != cachekey_timerids_.end()) {
        return iter->second;
    }
    return -1;
}
}
