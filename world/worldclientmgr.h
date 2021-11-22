#ifndef WORLD_WORLDCLIENTMGR_H
#define WORLD_WORLDCLIENTMGR_H

#include "shynet/utils/singleton.h"
#include "world/worldclient.h"

namespace world {
/// <summary>
/// 世界服连接管理器
/// </summary>
class WorldClientMgr final : public shynet::Nocopy {
    friend class shynet::utils::Singleton<WorldClientMgr>;
    WorldClientMgr();

public:
    ~WorldClientMgr();

    /*
		* 添加,删除,查找连接,k为连接fd
		*/
    void add(int k, std::shared_ptr<WorldClient> v);
    bool remove(int k);
    std::shared_ptr<WorldClient> find(int k);

    /// <summary>
    /// 所有连接列表,int为连接fd
    /// </summary>
    /// <returns></returns>
    std::unordered_map<int, std::shared_ptr<WorldClient>> clis() const;

    /*
	* 获取设置世界服务器监听地址
	*/
    const net::IPAddress& listen_addr() const
    {
        return listen_addr_;
    }
    void set_listen_addr(const net::IPAddress& addr)
    {
        listen_addr_ = addr;
    }

    /*
		* 负载均衡选择gamesid
		*/
    std::shared_ptr<WorldClient> select_game();

private:
    net::IPAddress listen_addr_;
    mutable std::mutex clis_mutex_;
    std::unordered_map<int, std::shared_ptr<WorldClient>> clis_;
};
}

#endif
