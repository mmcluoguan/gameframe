#include "login/loginclientmgr.h"
#include "frmpub/luacallbacktask.h"
#include "shynet/lua/luaengine.h"

namespace login {
LoginClientMgr::LoginClientMgr()
{
}

LoginClientMgr::~LoginClientMgr()
{
}

void LoginClientMgr::add(int k, std::shared_ptr<LoginClient> v)
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    clis_.insert({ k, v });

    //通知lua的onAccept函数
    shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
        std::make_shared<frmpub::OnAcceptTask<LoginClient>>(v));
}

bool LoginClientMgr::remove(int k)
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

std::shared_ptr<LoginClient> LoginClientMgr::find(int k)
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    auto it = clis_.find(k);
    if (it == clis_.end()) {
        return nullptr;
    }
    return it->second;
}

std::shared_ptr<LoginClient> LoginClientMgr::find_from_sid(const std::string& sid) const
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    for (auto&& [key, value] : clis_) {
        if (std::to_string(value->sif().sid()) == sid) {
            return value;
        }
    }
    return nullptr;
}

void LoginClientMgr::foreach_clis(std::function<void(int, std::shared_ptr<LoginClient>)> cb) const
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    for (auto&& [key, cli] : clis_) {
        cb(key, cli);
    }
}

const net::IPAddress& LoginClientMgr::listen_addr() const
{
    return listen_addr_;
}
void LoginClientMgr::set_listen_addr(const net::IPAddress& addr)
{
    listen_addr_ = addr;
}
}
