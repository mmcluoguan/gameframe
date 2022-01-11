#ifndef LOGIN_WORLDCONNECTOR_H
#define LOGIN_WORLDCONNECTOR_H

#include "frmpub/connector.h"

namespace login {
/**
 * @brief world服务器连接器
*/
class WorldConnector : public frmpub::Connector {
public:
    /**
     * @brief 构造
     * @param connect_addr world服务器地址
    */
    WorldConnector(std::shared_ptr<net::IPAddress> connect_addr);
    ~WorldConnector();

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
     * @brief 与world服务器断开连接回调
     * @param active 连接断开原因
    */
    void close(net::CloseType active) override;

private:
    /**
     * @brief 接收world服务器发来的错误信息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int errcode(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /**
     * @brief 转发消息到gate
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int forward_world_gate_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /// <summary>
    /// 接收world服务器发来的登录成功后world选择的gameid消息
    /// </summary>
    int login_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
};
}

#endif
