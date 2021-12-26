#ifndef DBVISIT_DBCLIENT_H
#define DBVISIT_DBCLIENT_H

#include "3rd/sw/redis++/redis++.h"
#include "frmpub/client.h"
#include "shynet/events/eventbuffer.h"
namespace redis = sw::redis;

namespace dbvisit {
/**
 * @brief db客户端连接
*/
class DbClient : public frmpub::Client, public std::enable_shared_from_this<DbClient> {
public:
    /**
     * @brief 构造
     * @param remote_addr db客户端地址
     * @param listen_addr 服务器监听地址
     * @param iobuf 管理io读写缓冲区
    */
    DbClient(std::shared_ptr<net::IPAddress> remote_addr,
        std::shared_ptr<net::IPAddress> listen_addr,
        std::shared_ptr<events::EventBuffer> iobuf);
    ~DbClient();

    /**
     * @brief 处理protobuf数据封包
     * @param obj protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int input_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<FilterData::Envelope>> enves) override;

    /**
     * @brief 客户端连接与db服务器断开连接回调
     * @param active 连接断开原因
    */
    void close(net::CloseType active) override;

private:
    /**
     * @brief 天
    */
    const std::chrono::seconds oneday_ = std::chrono::seconds(24 * 60 * 60);

private:
    /**
     * @brief 验证连接db服务器的客户端服务是否已经注册
     * @param sif 客户端服务信息
     * @return 是否已经注册
    */
    bool verify_register(const protocc::ServerInfo& sif);

    /**
     * @brief 接收db客户端发来的错误信息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int errcode(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /**
     * @brief 接收游戏服注册消息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return  0成功 -1失败 失败将关闭对端连接
    */
    int register_world_dbvisit_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /**
     * @brief 接收登录服注册消息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return  0成功 -1失败 失败将关闭对端连接
    */
    int register_login_dbvisit_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /**
     * @brief 接收游戏服注册消息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return  0成功 -1失败 失败将关闭对端连接
    */
    int register_game_dbvisit_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /**
     * @brief 接收网关服注册消息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return  0成功 -1失败 失败将关闭对端连接
    */
    int register_gate_dbvisit_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /**
     * @brief 接收加载1条hash数据消息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return  0成功 -1失败 失败将关闭对端连接
    */
    int loaddata_from_dbvisit_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /**
     * @brief 接收加载多条hash数据消息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return  0成功 -1失败 失败将关闭对端连接
    */
    int loaddata_more_from_dbvisit_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /**
     * @brief 接收添加1条hash数据消息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return  0成功 -1失败 失败将关闭对端连接
    */
    int insertdata_to_dbvisit_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /**
     * @brief 接收更新1条hash数据消息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return  0成功 -1失败 失败将关闭对端连接
    */
    int updata_to_dbvisit_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /**
     * @brief 接收删除1条hash数据消息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return  0成功 -1失败 失败将关闭对端连接
    */
    int deletedata_to_dbvisit_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /**
     * @brief 接收登录消息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return  0成功 -1失败 失败将关闭对端连接
    */
    int login_client_gate_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /**
     * @brief 接收玩家下线消息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return  0成功 -1失败 失败将关闭对端连接
    */
    int clioffline_gate_all_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /**
     * @brief 接收断线重连消息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return  0成功 -1失败 失败将关闭对端连接
    */
    int reconnect_client_gate_c(std::shared_ptr<protocc::CommonObject> data,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
};
}

#endif
