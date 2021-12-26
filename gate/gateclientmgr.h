#ifndef GATE_GATECLIENTMGR_H
#define GATE_GATECLIENTMGR_H

#include "gate/gateclient.h"
#include "shynet/utils/singleton.h"
#include <mutex>
#include <unordered_map>

namespace gate {
/**
 * @brief 连接gate的客户端管理器
*/
class GateClientMgr final : public shynet::Nocopy {
    friend class shynet::utils::Singleton<GateClientMgr>;
    GateClientMgr();

public:
    /**
    * @brief 类型名称
    */
    static constexpr const char* kClassname = "GateClientMgr";
    ~GateClientMgr();

    /**
     * @brief  添加连接game的客户端连接
     * @param k gate的客户端fd
     * @param v gate的客户端连接
    */
    void add(int k, std::shared_ptr<GateClient> v);
    /**
     * @brief 移除连接gate的客户端连接
     * @param k gate的客户端fd
    */
    bool remove(int k);
    /**
     * @brief 查找连接gate的客户端连接
     * @param k gate的客户端fd
     * @return 连接gate的客户端连接,找不到返回nullptr
    */
    std::shared_ptr<GateClient> find(int k);
    /**
     * @brief 查找连接game的客户端连接
     * @param key 账号或平台key查找
     * @return 连接gate的客户端连接,找不到返回nullptr
    */
    std::shared_ptr<GateClient> find(const std::string& key);

    /**
     * @brief 迭代所有管理的连接gate的客户端连接
     * @param cb 迭代函数
    */
    void foreach_clis(std::function<void(int, std::shared_ptr<GateClient>)> cb) const;

    /**
     * @brief 获取gate服务器监听地址
     * @return gate服务器监听地址
    */
    const net::IPAddress& listen_addr() const { return listen_addr_; }
    /**
     * @brief 设置gate服务器监听地址
     * @param addr gate服务器监听地址 
    */
    void set_listen_addr(const net::IPAddress& addr) { listen_addr_ = addr; }

private:
    /**
     * @brief  gate服务器监听地址 
    */
    net::IPAddress listen_addr_;
    /**
     * @brief 互斥体
    */
    mutable std::mutex clis_mutex_;
    /**
     * @brief 连接gate的客户端fd与连接gate的客户端映射的hash表
    */
    std::unordered_map<int, std::shared_ptr<GateClient>> clis_;
};
}

#endif
