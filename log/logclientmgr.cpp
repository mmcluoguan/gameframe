#include "log/logclientmgr.h"
#include "frmpub/luacallbacktask.h"

namespace logs {

void LogClientMgr::add(int k, std::shared_ptr<LogClient> v)
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    clis_.insert({ k, v });
}

void LogClientMgr::remove(int k)
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    clis_.erase(k);
}

std::shared_ptr<LogClient> LogClientMgr::find(int k)
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    auto it = clis_.find(k);
    if (it == clis_.end()) {
        return nullptr;
    }
    return it->second;
}

void LogClientMgr::foreach_clis(std::function<void(int, std::shared_ptr<LogClient>)> cb) const
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    for (auto&& [key, cli] : clis_) {
        cb(key, cli);
    }
}

}
