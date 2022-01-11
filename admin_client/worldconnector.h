#ifndef ADMIN_CLIENT_GATECONNECTOR_H
#define ADMIN_CLIENT_GATECONNECTOR_H

#include "frmpub/connector.h"

namespace admin_client {
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
     * @brief 与world服务器断开连接回调
     * @param active 连接断开原因
    */
    void close(net::CloseType active) override;

    /**
     * @brief 接收服务器发来的错误信息
     * @param doc josn文档
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int errcode(std::shared_ptr<rapidjson::Document> doc,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /**
     * @brief 接收服务器发来的区服信息
     * @param doc josn文档
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int getgamelist_admin_world_s(std::shared_ptr<rapidjson::Document> doc,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /**
     * @brief 接收服务器发来的广播通知消息结果
     * @param doc josn文档
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int noticeserver_admin_world_s(std::shared_ptr<rapidjson::Document> doc,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    /**
     * @brief 接收服务器发来的系统邮件消息结果
     * @param doc josn文档
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int sysemail_admin_world_s(std::shared_ptr<rapidjson::Document> doc,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

private:
    /*
    * @brief 游戏服id与游戏信息映射的hash表
    */
    std::unordered_map<int32_t, protocc::ServerInfo> gamemap_;
};
}

#endif
