#ifndef ADMIN_CLIENT_GATECONNECTOR_H
#define ADMIN_CLIENT_GATECONNECTOR_H

#include "frmpub/connector.h"

namespace admin_client {
class WorldConnector : public frmpub::Connector {
public:
    WorldConnector(std::shared_ptr<net::IPAddress> connect_addr);
    ~WorldConnector();

    void complete() override;
    int input_handle(std::shared_ptr<rapidjson::Document> doc, std::shared_ptr<std::stack<FilterData::Envelope>> enves) override;
    /// <summary>
    /// 与服务器连接断开回调
    /// </summary>
    /// <param name="active">断开原因</param>
    void close(net::ConnectEvent::CloseType active) override;

private:
    /*
    * 区服地图
    */
    std::unordered_map<int32_t, protocc::ServerInfo> gamemap_;

private:
    int errcode(std::shared_ptr<rapidjson::Document> doc,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    int getgamelist_admin_world_s(std::shared_ptr<rapidjson::Document> doc,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
    int noticeserver_admin_world_s(std::shared_ptr<rapidjson::Document> doc,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
};
}

#endif
