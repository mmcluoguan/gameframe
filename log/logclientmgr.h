#ifndef LOG_LOGCLIENTMGR_H
#define LOG_LOGCLIENTMGR_H

#include "log/logclient.h"
#include "shynet/utils/singleton.h"
#include <memory>
#include <mutex>
#include <unordered_map>

namespace logs {
/**
 * @brief 连接db的客户端管理器
*/
class LogClientMgr final : public shynet::Nocopy {
    friend class shynet::utils::Singleton<LogClientMgr>;

    LogClientMgr() = default;

public:
    /**
    * @brief 类型名称
    */
    static constexpr const char* kClassname = "LogClientMgr";
    ~LogClientMgr() = default;

    /**
     * @brief 添加连接log的客户端连接
     * @param k log的客户端fd
     * @param v log的客户端连接
    */
    void add(int k, std::shared_ptr<LogClient> v);
    /**
     * @brief 移除连接log的客户端连接
     * @param k log的客户端fd
    */
    void remove(int k);
    /**
     * @brief 查找连接log的客户端连接
     * @param k log的客户端fd
     * @return 连接log的客户端连接,找不到返回nullptr
    */
    std::shared_ptr<LogClient> find(int k);

    /**
     * @brief 迭代所有管理的连接log的客户端连接
     * @param cb 迭代函数
    */
    void foreach_clis(std::function<void(int, std::shared_ptr<LogClient>)> cb) const;

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
     * @brief 连接db的客户端fd与连接log的客户端映射的hash表
    */
    std::unordered_map<int, std::shared_ptr<LogClient>> clis_;
};
}

#endif
