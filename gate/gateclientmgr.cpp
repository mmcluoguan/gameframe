#include "gate/gateclientmgr.h"
#include "frmpub/luacallbacktask.h"
#include "shynet/lua/luaengine.h"

namespace gate {
GateClientMgr::GateClientMgr()
{
}

GateClientMgr::~GateClientMgr()
{
}

void GateClientMgr::add(int k, std::shared_ptr<GateClient> v)
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    clis_.insert({ k, v });
    //通知lua的onAccept函数
    shynet::utils::Singleton<lua::LuaEngine>::instance().append(
        std::make_shared<frmpub::OnAcceptTask<GateClient>>(v));
}

bool GateClientMgr::remove(int k)
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    if (clis_.erase(k) > 0) {

        //通知lua的onClose函数
        shynet::utils::Singleton<lua::LuaEngine>::instance().append(
            std::make_shared<frmpub::OnCloseTask>(k));

        return true;
    }
    return false;
}

std::shared_ptr<GateClient> GateClientMgr::find(int k)
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    auto it = clis_.find(k);
    if (it == clis_.end()) {
        return nullptr;
    }
    return it->second;
}

std::shared_ptr<GateClient> GateClientMgr::find(const std::string& key)
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    for (auto&& [k, value] : clis_) {
        if (value->accountid() == key || value->platform_key() == key) {
            return value;
        }
    }
    return nullptr;
}

void GateClientMgr::foreach_clis(std::function<void(int, std::shared_ptr<GateClient>)> cb) const
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    for (auto&& [key, cli] : clis_) {
        cb(key, cli);
    }
}

}
