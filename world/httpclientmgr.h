#ifndef WORLD_HTTPCLIENTMGR_H
#define WORLD_HTTPCLIENTMGR_H

#include "shynet/utils/singleton.h"
#include "world/httpclient.h"

namespace world {
/**
 * @brief 连接http的客户端管理器
*/
class HttpClientMgr final : public shynet::Nocopy {
    friend class shynet::utils::Singleton<HttpClientMgr>;
    HttpClientMgr() = default;

public:
    /**
    * @brief 类型名称
    */
    static constexpr const char* kClassname = "HttpClientMgr";
    ~HttpClientMgr() = default;

    /**
     * @brief  添加连接http的客户端连接
     * @param k http的客户端fd
     * @param v http的客户端连接
    */
    void add(int k, std::shared_ptr<HttpClient> v);
    /**
     * @brief 移除连接http的客户端连接
     * @param k http的客户端fd
    */
    bool remove(int k);
    /**
     * @brief 查找连接http的客户端连接
     * @param k http的客户端fd
     * @return 连接http的客户端连接,找不到返回nullptr
    */
    std::shared_ptr<HttpClient> find(int k);

    /**
     * @brief 迭代所有管理的连接http的客户端连接
     * @param cb 迭代函数
    */
    void foreach_clis(std::function<void(int, std::shared_ptr<HttpClient>)> cb) const;

    /**
     * @brief 获取http服务器监听地址
     * @return http服务器监听地址
    */
    const net::IPAddress& listen_addr() const { return listen_addr_; }
    /**
     * @brief 设置http服务器监听地址
     * @param addr http服务器监听地址 
    */
    void set_listen_addr(const net::IPAddress& addr) { listen_addr_ = addr; }

private:
    /**
     * @brief http服务器监听地址
    */
    net::IPAddress listen_addr_;
    /**
     * @brief 互斥体
    */
    mutable std::mutex clis_mutex_;
    /**
     * @brief 连接http的客户端fd与连接http的客户端映射的hash表
    */
    std::unordered_map<int, std::shared_ptr<HttpClient>> clis_;
};
}

#endif
