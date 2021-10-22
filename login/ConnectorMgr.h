#ifndef LOGIN_CONNECTORMGR_H
#define LOGIN_CONNECTORMGR_H

#include "login/dbconnector.h"
#include "login/worldconnector.h"
#include "shynet/utils/singleton.h"
#include <list>
#include <mutex>

namespace login {
/// <summary>
/// 服务器连接管理器
/// </summary>
class ConnectorMgr {
    friend class shynet::utils::Singleton<ConnectorMgr>;
    ConnectorMgr();

public:
    ~ConnectorMgr();

    std::shared_ptr<DbConnector> db_connector();
    void add_dbctor(int connectid);
    void remove_dbctor(int connectid);
    std::shared_ptr<DbConnector> find_dbctor(int connectid);

    std::shared_ptr<WorldConnector> world_connector();
    void add_worldctor(int connectid);
    void remove_worldctor(int connectid);
    std::shared_ptr<WorldConnector> find_worldctor(int connectid);

private:
    std::list<int> dbctor_ids_;
    std::mutex dbctor_ids_mtx_;
    std::list<int> worldctor_ids_;
    std::mutex worldctor_ids_mtx_;
};
}

#endif
