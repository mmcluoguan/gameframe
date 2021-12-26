#ifndef LOGIN_LOGINCLIENTMGR_H
#define LOGIN_LOGINCLIENTMGR_H

#include "login/loginclient.h"
#include "shynet/utils/singleton.h"
#include <unordered_map>

namespace login {
/**
 * @brief 连接login的客户端管理器
*/
class LoginClientMgr final : public shynet::Nocopy {
    friend class shynet::utils::Singleton<LoginClientMgr>;
    LoginClientMgr() = default;

public:
    /**
    * @brief 类型名称
    */
    static constexpr const char* kClassname = "LoginClientMgr";
    ~LoginClientMgr() = default;

    /**
     * @brief  添加连接login的客户端连接
     * @param k login的客户端fd
     * @param v login的客户端连接
    */
    void add(int k, std::shared_ptr<LoginClient> v);
    /**
     * @brief 移除连接login的客户端连接
     * @param k login的客户端fd
    */
    bool remove(int k);
    /**
     * @brief 查找连接login的客户端连接
     * @param k login的客户端fd
     * @return 连接login的客户端连接,找不到返回nullptr
    */
    std::shared_ptr<LoginClient> find(int k);
    /**
     * @brief 查找连接login的客户端连接
     * @param k login的客户端的服务id
     * @return 连接login的客户端连接,找不到返回nullptr
    */
    std::shared_ptr<LoginClient> find_from_sid(const std::string& sid) const;

    /**
     * @brief 迭代所有管理的连接login的客户端连接
     * @param cb 迭代函数
    */
    void foreach_clis(std::function<void(int, std::shared_ptr<LoginClient>)> cb) const;

    /**
     * @brief 获取login服务器监听地址
     * @return login服务器监听地址
    */
    const net::IPAddress& listen_addr() const { return listen_addr_; };
    /**
     * @brief 设置login服务器监听地址
     * @param addr login服务器监听地址 
    */
    void set_listen_addr(const net::IPAddress& addr) { listen_addr_ = addr; };

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
     * @brief 连接login的客户端fd与连接login的客户端映射的hash表
    */
    std::unordered_map<int, std::shared_ptr<LoginClient>> clis_;
};
}

#endif
