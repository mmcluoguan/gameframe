#ifndef ADMIN_CLIENT_CONNECTORMGR_H
#define ADMIN_CLIENT_CONNECTORMGR_H

#include "admin_client/worldconnector.h"
#include "shynet/utils/singleton.h"
#include <list>
#include <mutex>

namespace admin_client {
/// <summary>
/// 服务器连接管理器
/// </summary>
class ConnectorMgr {
    friend class shynet::utils::Singleton<ConnectorMgr>;
    ConnectorMgr();

public:
    ~ConnectorMgr();

    std::shared_ptr<WorldConnector> world_connector();
    void add_worldctor(int connectid);
    void remove_worldctor(int connectid);
    std::shared_ptr<WorldConnector> find_worldctor(int connectid);

private:
    std::list<int> worldctor_ids_;
    std::mutex worldctor_ids_mtx_;
};
}

#endif
