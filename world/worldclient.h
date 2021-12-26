#ifndef WORLD_WORLDCLIENT_H
#define WORLD_WORLDCLIENT_H

#include "frmpub/client.h"
#include "shynet/events/eventbuffer.h"

namespace world {
/**
 * @brief world客户端连接
*/
class WorldClient : public frmpub::Client, public std::enable_shared_from_this<WorldClient> {
public:
    /**
     * @brief 构造
     * @param remote_addr game客户端地址
     * @param listen_addr 服务器监听地址
     * @param iobuf 管理io读写缓冲区
    */
    WorldClient(std::shared_ptr<net::IPAddress> remote_addr,
        std::shared_ptr<net::IPAddress> listen_addr,
        std::shared_ptr<events::EventBuffer> iobuf);
    ~WorldClient();

    /**
     * @brief 处理protobuf数据封包
     * @param obj protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int input_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<FilterData::Envelope>> enves) override;

    /**
     * @brief 与world服务器断开连接回调
     * @param active 连接断开原因
    */
    void close(net::CloseType active) override;

    /**
     * @brief 获取连接数
     * @return 连接数
    */
    int connect_num() const { return connect_num_; }
    /**
     * @brief 设置连接数
     * @param value 连接数
    */
    void set_connect_num(int value) { connect_num_ = value; }

private:
    /**
     * @brief 接收world客户端发来的错误信息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int errcode(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /**
     * @brief 接收world客户端发来的登录服注册信息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int register_login_world_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /**
     * @brief 接收world客户端发来的游戏服注册信息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int register_game_world_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /**
     * @brief 接收world客户端发来的网关服注册信息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int register_gate_world_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /**
     * @brief 接收world客户端发来的玩家下线信息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int clioffline_gate_all_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /**
     * @brief 接收world客户端发来的成功后分配gamesid信息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int login_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

private:
    /**
     * @brief 连接数
    */
    int connect_num_ = 0;
};
}

#endif
