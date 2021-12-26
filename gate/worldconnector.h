#ifndef GATE_WORLDCONNECTOR_H
#define GATE_WORLDCONNECTOR_H

#include "frmpub/connector.h"

namespace gate {
/**
 * @brief world服务器连接器
*/
class WorldConnector : public frmpub::Connector, public std::enable_shared_from_this<WorldConnector> {
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
    int input_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<FilterData::Envelope>> enves) override;

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
     * @brief 接收服务器发来的网关服向世界服注册结果消息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return  0成功 -1失败 失败将关闭对端连接
    */
    int register_gate_world_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /**
     * @brief 接收服务器发来的其他服务器上线消息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int seronline_world_gate_g(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /**
     * @brief 转发消息到client
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int forward_world_client_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

private:
};
}

#endif
