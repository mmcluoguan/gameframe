#ifndef GATE_GATECLIENT_H
#define GATE_GATECLIENT_H

#include "frmpub/client.h"
#include "shynet/net/acceptnewfd.h"

namespace gate {
/// <summary>
/// 网关服客户端
/// </summary>
class GateClient : public frmpub::Client, public std::enable_shared_from_this<GateClient> {
public:
    GateClient(std::shared_ptr<net::IPAddress> remote_addr,
        std::shared_ptr<net::IPAddress> listen_addr,
        std::shared_ptr<events::EventBuffer> iobuf);
    ~GateClient();

    /// <summary>
    /// 处理网络消息
    /// </summary>
    /// <param name="obj">数据包</param>
    /// <param name="enves">数据包转发路由</param>
    /// <returns></returns>
    int input_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<FilterData::Envelope>> enves) override;

    /// <summary>
    /// 连接断开
    /// </summary>
    /// <param
    /// name="active">true服务器主动断开,false客户端主动断开</param>
    void close(bool active) override;

    /*
	* 获取设置玩家账号id
	*/
    void set_accountid(std::string t)
    {
        accountid_ = t;
    }
    std::string accountid() const
    {
        return accountid_;
    }
    /*
		* 获取设置玩家平台key
		*/
    void set_platform_key(std::string t)
    {
        platform_key_ = t;
    }
    std::string platform_key() const
    {
        return platform_key_;
    }

    /*
		* 获取设置选择的登录服id
		*/
    void set_login_id(int t)
    {
        login_id_ = t;
    }
    int login_id() const
    {
        return login_id_;
    }

    /*
		* 获取设置选择的游戏服id
		*/
    void set_game_id(int t)
    {
        game_id_ = t;
    }
    int game_id() const
    {
        return game_id_;
    }

private:
    /*
		* 到登陆消息处理
		*/
    int login_message(std::shared_ptr<protocc::CommonObject> obj,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /*
		* 到游戏消息处理
		*/
    int game_message(std::shared_ptr<protocc::CommonObject> obj,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /*
		* 返回服务器列表
		*/
    int serverlist_client_gate_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /*
		* 返回选择服务器结果
		*/
    int selectserver_client_gate_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

private:
    int login_id_ = 0;
    int game_id_ = 0;
    std::string accountid_;
    std::string platform_key_;
};
}

#endif
