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
    shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
        std::make_shared<frmpub::OnAcceptTask<GateClient>>(v));
}

bool GateClientMgr::remove(int k)
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

std::shared_ptr<GateClient> GateClientMgr::find(int k)
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    return clis_[k];
}

std::shared_ptr<GateClient> GateClientMgr::find(const std::string& key)
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    for (auto&& [k, value] : clis_) {
        if (value->set_accountid() == key || value->set_platform_key() == key) {
            return value;
        }
    }
    return nullptr;
}

std::unordered_map<int, std::shared_ptr<GateClient>> GateClientMgr::clis() const
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    return clis_;
}

}
