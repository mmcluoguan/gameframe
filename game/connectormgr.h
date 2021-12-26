#ifndef GAME_CONNECTORMGR_H
#define GAME_CONNECTORMGR_H

#include "game/dbconnector.h"
#include "game/worldconnector.h"
#include "shynet/utils/singleton.h"
#include <list>
#include <mutex>

namespace game {
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
     * @brief 获取db服务器连接器
     * @return db服务器连接器
    */
    std::shared_ptr<DbConnector> db_connector();
    /**
     * @brief 添加db服务器连接器id
     * @param connectid db服务器连接器id
    */
    void add_dbctor(int connectid);
    /**
     * @brief 移除db服务器连接器id
     * @param connectid db服务器连接器id
    */
    void remove_dbctor(int connectid);
    /**
     * @brief 查找db服务器连接器
     * @param connectid db服务器连接器id
     * @return db服务器连接器,找不到返回nullptr
    */
    std::shared_ptr<DbConnector> find_dbctor(int connectid);

    /**
     * @brief 获取world服务器连接器
     * @return world服务器连接器
    */
    std::shared_ptr<WorldConnector> world_connector();
    /**
     * @brief 添加world服务器连接器id
     * @param connectid world服务器连接器id
    */
    void add_worldctor(int connectid);
    /**
     * @brief 移除world服务器连接器id
     * @param connectid world服务器连接器id
    */
    void remove_worldctor(int connectid);
    /**
     * @brief 查找world服务器连接器
     * @param connectid world服务器连接器id
     * @return world服务器连接器,找不到返回nullptr
    */
    std::shared_ptr<WorldConnector> find_worldctor(int connectid);

private:
    /**
     * @brief db服务器连接器列表
    */
    std::list<int> dbctor_ids_;
    std::mutex dbctor_ids_mtx_;
    /**
     * @brief world服务器连接器列表
    */
    std::list<int> worldctor_ids_;
    std::mutex worldctor_ids_mtx_;
};
}

#endif
