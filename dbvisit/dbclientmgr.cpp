#include "dbvisit/dbclientmgr.h"
#include "frmpub/luacallbacktask.h"
#include "shynet/lua/luaengine.h"

namespace dbvisit {

void DbClientMgr::add(int k, std::shared_ptr<DbClient> v)
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    clis_.insert({ k, v });

    //通知lua的onAccept函数
    shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
        std::make_shared<frmpub::OnAcceptTask<DbClient>>(v));
}

void DbClientMgr::remove(int k)
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    if (clis_.erase(k) > 0) {

        //通知lua的onClose函数
        shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
            std::make_shared<frmpub::OnCloseTask>(k));
    }
}

std::shared_ptr<DbClient> DbClientMgr::find(int k)
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    auto it = clis_.find(k);
    if (it == clis_.end()) {
        return nullptr;
    }
    return it->second;
}

void DbClientMgr::foreach_clis(std::function<void(int, std::shared_ptr<DbClient>)> cb) const
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    for (auto&& [key, cli] : clis_) {
        cb(key, cli);
    }
}

std::shared_ptr<DbClient> DbClientMgr::find(std::string sid)
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    for (auto&& [key, value] : clis_) {
        if (std::to_string(value->sif().sid()) == sid) {
            return value;
        }
    }
    return nullptr;
}
}
