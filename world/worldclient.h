#ifndef WORLD_WORLDCLIENT_H
#define WORLD_WORLDCLIENT_H

#include "frmpub/client.h"
#include "shynet/events/eventbuffer.h"

namespace world {
/// <summary>
/// 世界服客户端
/// </summary>
class WorldClient : public frmpub::Client, public std::enable_shared_from_this<WorldClient> {
public:
    WorldClient(std::shared_ptr<net::IPAddress> remote_addr,
        std::shared_ptr<net::IPAddress> listen_addr,
        std::shared_ptr<events::EventBuffer> iobuf);
    ~WorldClient();

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
    /// <param name="active">true服务器主动断开,false客户端主动断开</param>
    void close(net::CloseType active) override;

    /*
		* 获取设置连接数
		*/
    int connect_num() const
    {
        return connect_num_;
    }
    void set_connect_num(int value)
    {
        connect_num_ = value;
    }

private:
    /// <summary>
    /// 服务器通用错误信息
    /// </summary>
    int errcode(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /*
		* 登录服注册
		*/
    int register_login_world_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /*
		* 游戏服注册
		*/
    int register_game_world_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /*
		* 网关服注册
		*/
    int register_gate_world_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /*
		* 玩家下线
		*/
    int clioffline_gate_all_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /*
		* 登录成功后分配gamesid
		*/
    int login_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

private:
    int connect_num_ = 0;
};
}

#endif
