#ifndef LOGIN_DBCONNECTOR_H
#define LOGIN_DBCONNECTOR_H

#include "frmpub/connector.h"

namespace login {
/**
 * @brief db服务器连接器
*/
class DbConnector : public frmpub::Connector {
public:
    /**
     * @brief 构造
     * @param connect_addr db服务器地址
    */
    DbConnector(std::shared_ptr<net::IPAddress> connect_addr);
    ~DbConnector();

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
     * @brief 与db服务器断开连接回调
     * @param active 连接断开原因
    */
    void close(net::CloseType active) override;

private:
    /**
     * @brief 接收db服务器发来的错误信息
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
    int forward_db_gate_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /**
     * @brief 接收db服务器发来的向db服注册结果消息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int register_login_dbvisit_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /**
     * @brief 接收db服务器发来的跨服顶号处理消息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int repeatlogin_client_gate_g(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /**
     * @brief 接收db服务器发来的登录结果消息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int login_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
};
}

#endif
