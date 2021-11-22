#include "dbvisit/dbclientmgr.h"
#include "frmpub/luacallbacktask.h"
#include "shynet/lua/luaengine.h"

namespace dbvisit {
DbClientMgr::DbClientMgr()
{
}

DbClientMgr::~DbClientMgr()
{
}

void DbClientMgr::add(int k, std::shared_ptr<DbClient> v)
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    clis_.insert({ k, v });

    //通知lua的onAccept函数
    shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
        std::make_shared<frmpub::OnAcceptTask<DbClient>>(v));
}

bool DbClientMgr::remove(int k)
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    if (clis_.erase(k) > 0) {

        //通知lua的onClose函数
        shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
            std::make_shared<frmpub::OnCloseTask>(k));

        return true;
    }
    return false;
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

std::unordered_map<int, std::shared_ptr<DbClient>> DbClientMgr::clis() const
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    return clis_;
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
