#include "world/worldclientmgr.h"
#include "frmpub/luacallbacktask.h"
#include "shynet/lua/luaengine.h"

namespace world {
WorldClientMgr::WorldClientMgr()
{
}

WorldClientMgr::~WorldClientMgr()
{
}

void WorldClientMgr::add(int k, std::shared_ptr<WorldClient> v)
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    clis_.insert({ k, v });

    //通知lua的onAccept函数
    shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
        std::make_shared<frmpub::OnAcceptTask<WorldClient>>(v));
}

bool WorldClientMgr::remove(int k)
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

std::shared_ptr<WorldClient> WorldClientMgr::find(int k)
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    auto it = clis_.find(k);
    if (it == clis_.end()) {
        return nullptr;
    }
    return it->second;
}

void WorldClientMgr::foreach_clis(std::function<void(int, std::shared_ptr<WorldClient>)> cb) const
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    for (auto&& [key, cli] : clis_) {
        cb(key, cli);
    }
}

std::shared_ptr<WorldClient> WorldClientMgr::select_game()
{
    std::lock_guard<std::mutex> lock(clis_mutex_);
    std::shared_ptr<WorldClient> game;
    int min = -1;
    for (auto&& [key, cli] : clis_) {
        if (cli->sif().st() == protocc::ServerType::GAME) {
            if (min == -1) {
                min = cli->connect_num();
                game = cli;
            } else if (cli->connect_num() < min) {
                min = cli->connect_num();
                game = cli;
            }
        }
    }
    if (game != nullptr) {

        game->set_connect_num(game->connect_num() + 1);
    }
    return game;
}
}
