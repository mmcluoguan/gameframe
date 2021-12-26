#ifndef GATE_CONNECTORMGR_H
#define GATE_CONNECTORMGR_H

#include "gate/gameconnector.h"
#include "gate/loginconnector.h"
#include "gate/worldconnector.h"
#include "shynet/utils/singleton.h"
#include <list>
#include <mutex>
#include <unordered_map>

namespace gate {
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
     * @brief 服务器连接信息
    */
    struct ConnectData {
        /**
         * @brief 服务器连接id
        */
        int connect_id = 0;
        /**
         * @brief 服务器信息
        */
        protocc::ServerInfo sif;
        /**
         * @brief 服务器接收数量
        */
        int connect_num = 0;
    };

public:
    /**
    * @brief 类型名称
    */
    static constexpr const char* kClassname = "ConnectorMgr";
    ~ConnectorMgr() = default;

    /*
	* 连接的服务器的服务id转换连接的服务器的连接id
	*/
    int sid_conv_connect_id(int sid);
    /**
     * @brief 添加新的服务器连接信息
     * @param connect_id 服务器的连接id
     * @param sif 新的服务器信息
    */
    void add_new_connect_data(int connect_id, protocc::ServerInfo sif);
    /**
     * @brief 移除服务器连接信息
     * @param connect_id 服务器的连接id
     * @return 是否成功
    */
    bool remove(int connect_id);
    /**
     * @brief 查找服务器连接信息
     * @param connect_id 服务器的连接id
     * @return 服务器连接信息,找不到返回nullptr
    */
    ConnectData* find_connect_data(int connect_id);

    /**
     * @brief 选择登录服务器，如果指定login_connect_id找不到,默认选择connect_num值最小的登录服务器
     * @param login_connect_id 登录服的连接id
     * @return 登录服连接器
    */
    std::shared_ptr<LoginConnector> select_login(int login_connect_id);
    /**
     * @brief 减少指定服务器的连接id中接收数量1
     * @param connect_id 服务器的连接id
     * @return 是否成功
    */
    bool reduce_count(int connect_id);

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

    /**
     * @brief 获取login服务器连接器
     * @param login_connect_id login服务器连接器id
     * @return 登录服务器连接器
    */
    std::shared_ptr<LoginConnector> login_connector(int login_connect_id) const;

    /**
     * @brief 获取game服务器连接器
     * @param game_connect_id game服务器连接器id
     * @return game服务器连接器
    */
    std::shared_ptr<GameConnector> game_connector(int game_connect_id) const;

    /**
     * @brief 迭代服务器连接信息hash表,key为conncet_id
     * @param cb 回调函数
    */
    void foreach_connect_datas(std::function<void(int, ConnectData)> cb) const;

private:
    /**
     * @brief world服务器连接器的连接id列表
    */
    std::list<int> worldctor_ids_;
    std::mutex worldctor_ids_mtx_;

    mutable std::mutex connect_data_mutex_;
    /**
     * @brief 服务器的连接id映射服务器连接信息的hase表
    */
    std::unordered_map<int, ConnectData> connect_datas_;
};
}

#endif
