#ifndef GAME_GAMECLIENT_H
#define GAME_GAMECLIENT_H

#include "frmpub/client.h"
#include "shynet/net/acceptnewfd.h"

namespace game {
/**
 * @brief game客户端连接
*/
class GameClient : public frmpub::Client {
public:
    /**
     * @brief 构造
     * @param remote_addr game客户端地址
     * @param listen_addr 服务器监听地址
     * @param iobuf 管理io读写缓冲区
    */
    GameClient(std::shared_ptr<net::IPAddress> remote_addr,
        std::shared_ptr<net::IPAddress> listen_addr,
        std::shared_ptr<events::EventBuffer> iobuf);
    ~GameClient();

    /**
     * @brief 处理protobuf数据封包
     * @param obj protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int default_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<FilterData::Envelope>> enves) override;

    /**
     * @brief 客户端连接与game服务器断开连接回调
     * @param active 连接断开原因
    */
    void close(net::CloseType active) override;

private:
    /**
     * @brief 接收game客户端发来的错误信息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int errcode(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /**
     * @brief 接收网关服注册消息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int register_gate_game_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
};
}

#endif
