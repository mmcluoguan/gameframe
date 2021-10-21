#ifndef CLIENT_GATECONNECTOR_H
#define CLIENT_GATECONNECTOR_H

#include "frmpub/Connector.h"

namespace client {
class GateConnector : public frmpub::Connector {
public:
    /// <summary>
    /// 断线重连数据
    /// </summary>
    struct DisConnectData {
        int login_id = 0;
        int game_id = 0;
        std::string accountid;
    };

    GateConnector(std::shared_ptr<net::IPAddress> connect_addr,
        std::shared_ptr<DisConnectData> disconnect = nullptr);
    ~GateConnector();

    void complete() override;
    int input_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<FilterData::Envelope>> enves) override;

    /// <summary>
    /// 获取断线前的连接数据
    /// </summary>
    /// <returns></returns>
    std::shared_ptr<GateConnector::DisConnectData> disconnect_data();

private:
    /// <summary>
    /// 服务器通用错误信息
    /// </summary>
    int errcode(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /*
		* 请求服务器列表结果
		*/
    int serverlist_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /*
		* 请求选择服务器结果
		*/
    int selectserver_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /*
		* 请求登录结果
		*/
    int login_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /*
		* 被顶号通知
		*/
    int repeatlogin_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /*
		* 请求重连服务器结果
		*/
    int reconnect_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /*
		* 创建角色服务器结果
		*/
    int createrole_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /*
		* 加载角色数据结果
		*/
    int loadrole_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /*
		* 加载角色物品数据结果
		*/
    int loadgoods_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

private:
    int login_id_ = 0;
    int game_id_ = 0;
    bool enable_reconnect_ = true;
    std::string accountid_;
    int64_t roleid_;
    std::shared_ptr<DisConnectData> disconnect_ = nullptr;
};
}

#endif
