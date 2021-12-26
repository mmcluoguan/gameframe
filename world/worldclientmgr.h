#ifndef WORLD_WORLDCLIENTMGR_H
#define WORLD_WORLDCLIENTMGR_H

#include "shynet/utils/singleton.h"
#include "world/worldclient.h"

namespace world {
/**
 * @brief 连接world的客户端管理器
*/
class WorldClientMgr final : public shynet::Nocopy {
    friend class shynet::utils::Singleton<WorldClientMgr>;
    WorldClientMgr() = default;

public:
    /**
    * @brief 类型名称
    */
    static constexpr const char* kClassname = "WorldClientMgr";
    ~WorldClientMgr() = default;

    /**
     * @brief  添加连接world的客户端连接
     * @param k world的客户端fd
     * @param v world的客户端连接
    */
    void add(int k, std::shared_ptr<WorldClient> v);
    /**
     * @brief 移除连接world的客户端连接
     * @param k world的客户端fd
    */
    bool remove(int k);
    /**
     * @brief 查找连接world的客户端连接
     * @param k world的客户端fd
     * @return 连接world的客户端连接,找不到返回nullptr
    */
    std::shared_ptr<WorldClient> find(int k);

    /**
     * @brief 迭代所有管理的连接world的客户端连接
     * @param cb 迭代函数
    */
    void foreach_clis(std::function<void(int, std::shared_ptr<WorldClient>)> cb) const;

    /**
     * @brief 获取world服务器监听地址
     * @return world服务器监听地址
    */
    const net::IPAddress& listen_addr() const { return listen_addr_; }
    /**
     * @brief 设置world服务器监听地址
     * @param addr world服务器监听地址 
    */
    void set_listen_addr(const net::IPAddress& addr) { listen_addr_ = addr; }
    /**
     * @brief 负载均衡选择world的客户端连接
     * @return world的客户端连接
    */
    std::shared_ptr<WorldClient> select_game();

private:
    /**
     * @brief world服务器监听地址
    */
    net::IPAddress listen_addr_;
    /**
     * @brief 互斥体
    */
    mutable std::mutex clis_mutex_;
    /**
     * @brief 连接world的客户端fd与连接world的客户端映射的hash表
    */
    std::unordered_map<int, std::shared_ptr<WorldClient>> clis_;
};
}

#endif
