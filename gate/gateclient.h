#ifndef GATE_GATECLIENT_H
#define GATE_GATECLIENT_H

#include "frmpub/client.h"
#include "shynet/net/acceptnewfd.h"

namespace gate {
/**
 * @brief gate客户端连接
*/
class GateClient : public frmpub::Client, public std::enable_shared_from_this<GateClient> {
public:
    /**
     * @brief 构造
     * @param remote_addr game客户端地址
     * @param listen_addr 服务器监听地址
     * @param iobuf 管理io读写缓冲区
    */
    GateClient(std::shared_ptr<net::IPAddress> remote_addr,
        std::shared_ptr<net::IPAddress> listen_addr,
        std::shared_ptr<events::EventBuffer> iobuf);
    ~GateClient();

    /**
     * @brief 处理protobuf数据封包
     * @param obj protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int input_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<FilterData::Envelope>> enves) override;

    /**
     * @brief 客户端连接与gate服务器断开连接回调
     * @param active 连接断开原因
    */
    void close(net::CloseType active) override;

    /**
     * @brief 设置玩家账号id
     * @param t 玩家账号id
    */
    void set_accountid(std::string t) { accountid_ = t; }
    /**
     * @brief 获取玩家账号id
     * @return 玩家账号id
    */
    std::string accountid() const { return accountid_; }
    /**
     * @brief 设置玩家平台key
     * @param t 玩家平台key
    */
    void set_platform_key(std::string t) { platform_key_ = t; }
    /**
     * @brief 获取玩家平台key
     * @return 玩家平台key
    */
    std::string platform_key() const { return platform_key_; }

    /**
     * @brief 设置选择的登录服id
     * @param t 选择的登录服id
    */
    void set_login_id(int t) { login_id_ = t; }
    /**
     * @brief 获取选择的登录服id
     * @return 选择的登录服id
    */
    int login_id() const { return login_id_; }
    /**
     * @brief 设置选择的游戏服id
     * @param t 选择的游戏服id
    */
    void set_game_id(int t) { game_id_ = t; }
    /**
     * @brief 获取选择的游戏服id
     * @return 选择的游戏服id
    */
    int game_id() const { return game_id_; }

private:
    /**
     * @brief 到login服务器消息预处理
     * @param obj protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int login_message(std::shared_ptr<protocc::CommonObject> obj,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /**
     * @brief 到game服务器消息预处理
     * @param obj protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int game_message(std::shared_ptr<protocc::CommonObject> obj,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /**
     * @brief 接收客户端发来的获取服务器列表信息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int serverlist_client_gate_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /**
     * @brief 接收客户端发来的选择服务器信息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int selectserver_client_gate_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

private:
    /**
     * @brief 登录服id
    */
    int login_id_ = 0;
    /**
     * @brief 游戏服id
    */
    int game_id_ = 0;
    /**
     * @brief 玩家账号id
    */
    std::string accountid_;
    /**
     * @brief 玩家平台key
    */
    std::string platform_key_;
};
}

#endif
