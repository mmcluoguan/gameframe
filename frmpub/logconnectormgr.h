#ifndef FRMPUB_LOGCONNECTORMGR_H
#define FRMPUB_LOGCONNECTORMGR_H

#include "frmpub/logconnector.h"
#include "shynet/utils/singleton.h"
#include <list>
#include <mutex>

namespace frmpub {
/**
 * @brief 服务器连接管理器
*/
class LogConnectorMgr {
    friend class shynet::utils::Singleton<LogConnectorMgr>;
    /**
     * @brief 构造
    */
    LogConnectorMgr() = default;

public:
    /**
    * @brief 类型名称
    */
    static constexpr const char* kClassname = "LogConnectorMgr";
    ~LogConnectorMgr() = default;

    /**
     * @brief 获取log服务器连接器
     * @return log服务器连接器
    */
    std::shared_ptr<LogConnector> log_connector();
    /**
     * @brief 添加log服务器连接器id
     * @param connectid log服务器连接器id
    */
    void add_logctor(int connectid);
    /**
     * @brief 移除log服务器连接器id
     * @param connectid log服务器连接器id
    */
    void remove_logctor(int connectid);
    /**
     * @brief 查找log服务器连接器
     * @param connectid log服务器连接器id
     * @return log服务器连接器,找不到返回nullptr
    */
    std::shared_ptr<LogConnector> find_logctor(int connectid);

private:
    /**
     * @brief log服务器连接器列表
    */
    std::list<int> logctor_ids_;
    std::mutex logctor_ids_mtx_;
};
}

#endif
