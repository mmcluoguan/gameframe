#ifndef CLIENT_GATECONNECTOR_H
#define CLIENT_GATECONNECTOR_H

#include "client/role.h"
#include "frmpub/connector.h"

namespace client {
/**
 * @brief gate服务器连接器
*/
class GateConnector : public frmpub::Connector, public std::enable_shared_from_this<GateConnector> {
public:
    /**
     * @brief 重连前的连接数据
    */
    struct DisConnectData {
        /**
         * @brief 选择的loginid
        */
        int login_id = 0;
        /**
         * @brief 选择的gameid
        */
        int game_id = 0;
        /**
         * @brief 账号id
        */
        std::string accountid;
        /**
         * @brief 角色id
        */
        int64_t roleid = 0;
    };

    /**
     * @brief 构造
     * @param connect_addr world服务器地址
    */
    GateConnector(std::shared_ptr<net::IPAddress> connect_addr,
        std::shared_ptr<DisConnectData> disconnect = nullptr);
    ~GateConnector();

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
     * @brief 获取重连前的连接数据
     * @return 重连的连接数据
    */
    std::shared_ptr<GateConnector::DisConnectData> disconnect_data();

    /**
     * @brief 设置平台key
     * @param key 平台key
    */
    void set_platform_key(std::string key) { platform_key_ = key; }

    /**
     * @brief 获取角色
     * @return 角色
    */
    std::shared_ptr<Role> role() { return role_; };

    /**
     * @brief 接收gate服务器发来的错误信息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int errcode(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /**
     * @brief 接收gate服务器发来的服务器列表信息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int serverlist_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /**
     * @brief 接收gate服务器发来的选择服务器结果信息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int selectserver_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /**
     * @brief 接收gate服务器发来的登录结果信息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int login_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /**
     * @brief 接收gate服务器发来的被顶号通知信息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int repeatlogin_client_gate_g(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /**
     * @brief 接收gate服务器发来的重连服务器结果信息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int reconnect_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /**
     * @brief 接收gate服务器发来的创建角色结果信息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int createrole_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /**
     * @brief 接收gate服务器发来的gm操作结果信息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int gmorder_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

private:
    /**
     * @brief 平台key
    */
    std::string platform_key_ = "aaaa_123456";
    /**
     * @brief loginid
    */
    int login_id_ = 0;
    /**
     * @brief gameid
    */
    int game_id_ = 0;
    /**
     * @brief 是否自动断线重连
    */
    bool enable_reconnect_ = true;
    /**
     * @brief 账号id
    */
    std::string accountid_;
    /**
     * @brief 重连前的连接数据
    */
    std::shared_ptr<DisConnectData> disconnect_ = nullptr;
    /**
     * @brief 角色
    */
    std::shared_ptr<Role> role_ = nullptr;
};
}

#endif
