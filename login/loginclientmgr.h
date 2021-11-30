#ifndef LOGIN_LOGINCLIENTMGR_H
#define LOGIN_LOGINCLIENTMGR_H

#include "login/loginclient.h"
#include "shynet/utils/singleton.h"
#include <unordered_map>

namespace login {
/// <summary>
/// 登录服连接管理器
/// </summary>
class LoginClientMgr final : public shynet::Nocopy {
    friend class shynet::utils::Singleton<LoginClientMgr>;
    LoginClientMgr();

public:
    ~LoginClientMgr();

    /*
		* 添加,删除,查找连接,k为连接fd
		*/
    void add(int k, std::shared_ptr<LoginClient> v);
    bool remove(int k);
    std::shared_ptr<LoginClient> find(int k);
    /*
		* 通过服务id查找连接
		*/
    std::shared_ptr<LoginClient> find_from_sid(const std::string& sid) const;

    /// <summary>
    /// 迭代所有连接列表
    /// </summary>
    /// <param name="cb"></param>
    void foreach_clis(std::function<void(int, std::shared_ptr<LoginClient>)> cb) const;

    /*
		* 获取设置登录服务器监听地址
		*/
    const net::IPAddress& listen_addr() const;
    void set_listen_addr(const net::IPAddress& addr);

private:
    net::IPAddress listen_addr_;
    mutable std::mutex clis_mutex_;
    std::unordered_map<int, std::shared_ptr<LoginClient>> clis_;
};
}

#endif
