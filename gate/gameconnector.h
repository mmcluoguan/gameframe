#ifndef GATE_GAMECONNECTOR_H
#define GATE_GAMECONNECTOR_H

#include "frmpub/connector.h"

namespace gate {
/**
 * @brief game服务器连接器
*/
class GameConnector : public frmpub::Connector {
public:
    /**
     * @brief 构造
     * @param connect_addr world服务器地址
    */
    GameConnector(std::shared_ptr<net::IPAddress> connect_addr);
    ~GameConnector();

    /**
     * @brief 连接完成回调
    */
    void complete() override;

    /**
     * @brief 处理protobuf数据封包
     * @param obj protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int default_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<FilterData::Envelope>> enves) override;

    /**
     * @brief 与game服务器断开连接回调
     * @param active 连接断开原因
    */
    void close(net::CloseType active) override;

    /**
     * @brief 获取连接id
     * @param v 连接id
    */
    void game_conncet_id(int v) { game_connect_id_ = v; }
    /**
     * @brief 设置连接id
     * @return 连接id
    */
    int set_game_conncet_id() const { return game_connect_id_; }

private:
    /**
     * @brief 接收game服务器发来的错误信息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int errcode(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /**
     * @brief 转发消息到client
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int forward_game_client_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

private:
    /**
     * @brief 连接id
    */
    int game_connect_id_;
};
}

#endif
