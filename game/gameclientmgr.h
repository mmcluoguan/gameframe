#ifndef GAME_GAMECLIENTMGR_H
#define GAME_GAMECLIENTMGR_H

#include "game/gameclient.h"
#include "shynet/utils/singleton.h"
#include <unordered_map>

namespace game {
/**
 * @brief 连接game的客户端管理器
*/
class GameClientMgr final : public shynet::Nocopy {
    friend class shynet::utils::Singleton<GameClientMgr>;
    GameClientMgr() = default;

public:
    /**
    * @brief 类型名称
    */
    static constexpr const char* kClassname = "GameClientMgr";
    ~GameClientMgr() = default;

    /**
     * @brief  添加连接game的客户端连接
     * @param k game的客户端fd
     * @param v game的客户端连接
    */
    void add(int k, std::shared_ptr<GameClient> v);
    /**
     * @brief 移除连接game的客户端连接
     * @param k game的客户端fd
    */
    bool remove(int k);
    /**
     * @brief 查找连接game的客户端连接
     * @param k game的客户端fd
     * @return 连接game的客户端连接,找不到返回nullptr
    */
    std::shared_ptr<GameClient> find(int k);

    /**
     * @brief 迭代所有管理的连接game的客户端连接
     * @param cb 迭代函数
    */
    void foreach_clis(std::function<void(int, std::shared_ptr<GameClient>)> cb) const;

    /**
     * @brief 获取game服务器监听地址
     * @return game服务器监听地址
    */
    const net::IPAddress& listen_addr() const { return listen_addr_; }
    /**
     * @brief 设置game服务器监听地址
     * @param addr game服务器监听地址 
    */
    void set_listen_addr(const net::IPAddress& addr) { listen_addr_ = addr; }

private:
    /**
     * @brief  game服务器监听地址 
    */
    net::IPAddress listen_addr_;
    /**
     * @brief 互斥体
    */
    mutable std::mutex clis_mutex_;
    /**
     * @brief 连接game的客户端fd与连接game的客户端映射的hash表
    */
    std::unordered_map<int, std::shared_ptr<GameClient>> clis_;
};
}

#endif
