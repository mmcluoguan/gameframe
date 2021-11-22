#ifndef WORLD_HTTPCLIENTMGR_H
#define WORLD_HTTPCLIENTMGR_H

#include "shynet/utils/singleton.h"
#include "world/httpclient.h"

namespace world {
/// <summary>
/// http后台连接管理器
/// </summary>
class HttpClientMgr final : public shynet::Nocopy {
    friend class shynet::utils::Singleton<HttpClientMgr>;
    HttpClientMgr();

public:
    ~HttpClientMgr();

    /*
		* 添加,删除,查找连接,k为连接fd
		*/
    void add(int k, std::shared_ptr<HttpClient> v);
    bool remove(int k);
    std::shared_ptr<HttpClient> find(int k);

    /// <summary>
    /// 所有连接列表,int为连接fd
    /// </summary>
    /// <returns></returns>
    std::unordered_map<int, std::shared_ptr<HttpClient>> clis() const;

    /*
	* 获取设置http后台服务器监听地址
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
    std::unordered_map<int, std::shared_ptr<HttpClient>> clis_;
};
}

#endif
