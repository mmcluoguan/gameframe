#ifndef WORLD_CONNECTORMGR_H
#define WORLD_CONNECTORMGR_H

#include "shynet/utils/singleton.h"
#include "world/dbconnector.h"
#include <list>

namespace world {
/// <summary>
/// 服务器连接管理器
/// </summary>
class ConnectorMgr {
    friend class shynet::utils::Singleton<ConnectorMgr>;
    ConnectorMgr() = default;

public:
    /**
    * @brief 类型名称
    */
    static constexpr const char* kClassname = "ConnectorMgr";

    ~ConnectorMgr() = default;

    std::shared_ptr<DbConnector> db_connector();

    void add_dbctor(int connectid);
    void remove_dbctor(int connectid);
    std::shared_ptr<DbConnector> find_dbctor(int connectid);

private:
    std::list<int> dbctor_ids_;
    std::mutex dbctor_ids_mtx_;
};
}

#endif
