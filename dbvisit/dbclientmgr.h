#ifndef DBVISIT_DBCLIENTMGR_H
#define DBVISIT_DBCLIENTMGR_H

#include "dbvisit/dbclient.h"
#include "shynet/utils/singleton.h"
#include <memory>
#include <mutex>
#include <unordered_map>

namespace dbvisit {
/**
 * @brief 连接db的客户端管理器
*/
class DbClientMgr final : public shynet::Nocopy {
    friend class shynet::utils::Singleton<DbClientMgr>;

    DbClientMgr() = default;

public:
    /**
    * @brief 类型名称
    */
    static constexpr const char* kClassname = "DbClientMgr";
    ~DbClientMgr() = default;

    /**
     * @brief 添加连接db的客户端连接
     * @param k db的客户端fd
     * @param v db的客户端连接
    */
    void add(int k, std::shared_ptr<DbClient> v);
    /**
     * @brief 移除连接db的客户端连接
     * @param k db的客户端fd
    */
    void remove(int k);
    /**
     * @brief 查找连接db的客户端连接
     * @param k db的客户端fd
     * @return 连接db的客户端连接,找不到返回nullptr
    */
    std::shared_ptr<DbClient> find(int k);
    /**
     * @brief 查找连接db的客户端连接
     * @param sid  连接db的客户端的服务器id 
    (连接db的客户端都是应用服务器)
     * @return 连接db的客户端连接,找不到返回nullptr
    */
    std::shared_ptr<DbClient> find(std::string sid);

    /**
     * @brief 迭代所有管理的连接db的客户端连接
     * @param cb 迭代函数
    */
    void foreach_clis(std::function<void(int, std::shared_ptr<DbClient>)> cb) const;

    /**
     * @brief 获取db服务器监听地址
     * @return db服务器监听地址
    */
    const net::IPAddress& listen_addr() const { return listen_addr_; }
    /**
     * @brief 设置db服务器监听地址
     * @param addr db服务器监听地址 
    */
    void set_listen_addr(const net::IPAddress& addr) { listen_addr_ = addr; }

private:
    /**
     * @brief  db服务器监听地址 
    */
    net::IPAddress listen_addr_;
    /**
     * @brief 互斥体
    */
    mutable std::mutex clis_mutex_;
    /**
     * @brief 连接db的客户端fd与连接db的客户端映射的hash表
    */
    std::unordered_map<int, std::shared_ptr<DbClient>> clis_;
};
}

#endif
