#ifndef LOGIN_DBCONNECTOR_H
#define LOGIN_DBCONNECTOR_H

#include "frmpub/connector.h"

namespace login {
/*
	 * db连接器
	 */
class DbConnector : public frmpub::Connector, public std::enable_shared_from_this<DbConnector> {
public:
    DbConnector(std::shared_ptr<net::IPAddress> connect_addr);
    ~DbConnector();

    /// <summary>
    /// 连接完成
    /// </summary>
    void complete() override;

    /// <summary>
    /// 处理网络消息
    /// </summary>
    /// <param name="obj">数据包</param>
    /// <param name="enves">数据包转发路由</param>
    /// <returns></returns>
    int input_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<FilterData::Envelope>> enves) override;

    /// <summary>
    /// 与服务器连接断开回调
    /// </summary>
    /// <param name="active">断开原因</param>
    void close(net::ConnectEvent::CloseType active) override;

private:
    /// <summary>
    /// 服务器通用错误信息
    /// </summary>
    int errcode(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /// <summary>
    /// 登录服向db服注册结果
    /// </summary>
    int register_login_dbvisit_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /// <summary>
    /// 跨服顶号处理
    /// </summary>
    int repeatlogin_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /// <summary>
    /// 登录结果
    /// </summary>
    int login_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /// <summary>
    /// 转发消息到gate
    /// </summary>
    int forward_client_gate_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
};
}

#endif
