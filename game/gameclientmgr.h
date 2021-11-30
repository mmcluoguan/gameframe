#ifndef GAME_GAMECLIENTMGR_H
#define GAME_GAMECLIENTMGR_H

#include "game/gameclient.h"
#include "shynet/utils/singleton.h"
#include <unordered_map>

namespace game {
/// <summary>
/// 游戏服连接管理器
/// </summary>
class GameClientMgr final : public shynet::Nocopy {
    friend class shynet::utils::Singleton<GameClientMgr>;
    GameClientMgr();

public:
    ~GameClientMgr();

    /*
		* 添加,删除,查找连接,k为连接fd
		*/
    void add(int k, std::shared_ptr<GameClient> v);
    bool remove(int k);
    std::shared_ptr<GameClient> find(int k);

    /// <summary>
    /// 迭代所有连接列表
    /// </summary>
    /// <param name="cb"></param>
    void foreach_clis(std::function<void(int, std::shared_ptr<GameClient>)> cb) const;

    /*
	* 获取设置游戏服务器监听地址
	*/
    const net::IPAddress& listen_addr() const
    {
        return listen_addr_;
    }
    void set_listen_addr(const net::IPAddress& addr)
    {
        listen_addr_ = addr;
    }

private:
    net::IPAddress listen_addr_;
    mutable std::mutex clis_mutex_;
    std::unordered_map<int, std::shared_ptr<GameClient>> clis_;
};
}

#endif
