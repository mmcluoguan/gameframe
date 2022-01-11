#ifndef LOGIN_LOGINCLIENT_H
#define LOGIN_LOGINCLIENT_H

#include "frmpub/client.h"
#include "shynet/net/acceptnewfd.h"

namespace login {
/**
 * @brief login客户端连接
*/
class LoginClient : public frmpub::Client {
public:
    /**
     * @brief 构造
     * @param remote_addr game客户端地址
     * @param listen_addr 服务器监听地址
     * @param iobuf 管理io读写缓冲区
    */
    LoginClient(std::shared_ptr<net::IPAddress> remote_addr,
        std::shared_ptr<net::IPAddress> listen_addr,
        std::shared_ptr<events::EventBuffer> iobuf);
    ~LoginClient();

    /**
     * @brief 处理protobuf数据封包
     * @param obj protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int default_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<FilterData::Envelope>> enves) override;

    /**
     * @brief 客户端连接与login服务器断开连接回调
     * @param active 连接断开原因
    */
    void close(net::CloseType active) override;

private:
    /**
     * @brief 接收login客户端发来的错误信息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int errcode(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /**
     * @brief 接收login客户端发来的网关服注册消息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int register_gate_login_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /**
     * @brief 接收login客户端发来的登录消息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int login_client_gate_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /**
     * @brief 转发db服处理
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int forward_client_gate_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /**
     * @brief 接收login客户端发来的玩家下线消息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int clioffline_gate_all_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /**
     * @brief 玩家login客户端发来的创建角色成功消息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int createrole_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
};
}

#endif
