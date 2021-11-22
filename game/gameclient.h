#ifndef GAME_GAMECLIENT_H
#define GAME_GAMECLIENT_H

#include "frmpub/client.h"
#include "shynet/net/acceptnewfd.h"

namespace game {
/// <summary>
/// 游戏服客户端
/// </summary>
class GameClient : public frmpub::Client, public std::enable_shared_from_this<GameClient> {
public:
    GameClient(std::shared_ptr<net::IPAddress> remote_addr,
        std::shared_ptr<net::IPAddress> listen_addr,
        std::shared_ptr<events::EventBuffer> iobuf);
    ~GameClient();

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

private:
    /// <summary>
    /// 服务器通用错误信息
    /// </summary>
    int errcode(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /*
		* 网关服注册
		*/
    int register_gate_game_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
};
}

#endif
