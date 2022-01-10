#ifndef FRMPUB_LOGCONNECTOR_H
#define FRMPUB_LOGCONNECTOR_H

#include "frmpub/connector.h"

namespace frmpub {
/**
 * @brief log服务器连接器
*/
class LogConnector : public frmpub::Connector {
public:
    /**
     * @brief 构造
     * @param connect_addr db服务器地址
    */
    LogConnector(std::shared_ptr<net::IPAddress> connect_addr);
    ~LogConnector();

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
     * @brief 与log服务器断开连接回调
     * @param active 连接断开原因
    */
    void close(net::CloseType active) override;

    /**
     * @brief 接收log服务器发来的错误信息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int errcode(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
};
}

#endif
