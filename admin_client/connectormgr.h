#ifndef ADMIN_CLIENT_CONNECTORMGR_H
#define ADMIN_CLIENT_CONNECTORMGR_H

#include "admin_client/worldconnector.h"
#include "shynet/utils/singleton.h"
#include <list>
#include <mutex>

namespace admin_client {
/**
 * @brief 服务器连接管理器
*/
class ConnectorMgr {
    friend class shynet::utils::Singleton<ConnectorMgr>;
    /**
     * @brief 构造
    */
    ConnectorMgr() = default;

public:
    /**
    * @brief 类型名称
    */
    static constexpr const char* kClassname = "ConnectorMgr";
    ~ConnectorMgr() = default;

    /**
     * @brief 获取world服务器连接器
     * @return world服务器连接器
    */
    std::shared_ptr<WorldConnector> world_connector();
    /**
     * @brief 添加world服务器连接器连接id
     * @param connectid world服务器连接器连接id
    */
    void add_worldctor(int connectid);
    /**
     * @brief 移除world服务器连接器连接id
     * @param connectid world服务器连接器连接id
    */
    void remove_worldctor(int connectid);
    /**
     * @brief 查找world服务器连接器
     * @param connectid world服务器连接器连接id
     * @return world服务器连接器,找不到返回nullptr
    */
    std::shared_ptr<WorldConnector> find_worldctor(int connectid);

private:
    /**
     * @brief world服务器连接器连接id的列表
    */
    std::list<int> worldctor_ids_;
    /**
     * @brief 互斥体
    */
    std::mutex worldctor_ids_mtx_;
};
}

#endif
