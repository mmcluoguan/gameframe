#include "world/httpclientmgr.h"
#include "frmpub/luacallbacktask.h"
#include "shynet/lua/luaengine.h"

namespace world {
HttpClientMgr::HttpClientMgr()
{
}

HttpClientMgr::~HttpClientMgr()
{
}

void HttpClientMgr::add(int k, std::shared_ptr<HttpClient> v)
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    clis_.insert({ k, v });

    //通知lua的onAccept函数
    shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
        std::make_shared<frmpub::OnAcceptTask<HttpClient>>(v));
}

bool HttpClientMgr::remove(int k)
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

std::shared_ptr<HttpClient> HttpClientMgr::find(int k)
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    auto it = clis_.find(k);
    if (it == clis_.end()) {
        return nullptr;
    }
    return it->second;
}

void HttpClientMgr::foreach_clis(std::function<void(int, std::shared_ptr<HttpClient>)> cb) const
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    for (auto&& [key, cli] : clis_) {
        cb(key, cli);
    }
}

}
