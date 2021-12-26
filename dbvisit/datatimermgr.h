#ifndef DBVISIT_DATATIMERMGR_H
#define DBVISIT_DATATIMERMGR_H

#include "shynet/utils/singleton.h"
#include <unordered_map>

namespace dbvisit {
/**
 * @brief 延迟更新数据计时器id管理器
*/
class DataTimerMgr : public shynet::Nocopy {
    friend class shynet::utils::Singleton<DataTimerMgr>;
    DataTimerMgr() = default;

public:
    /**
    * @brief 类型名称
    */
    static constexpr const char* kClassname = "DataTimerMgr";
    ~DataTimerMgr() = default;

    /**
     * @brief 添加延迟更新数据计时器
     * @param cachekey cachekey定义规则为tablename_key 
     * @param timerid 计时器id
    */
    void add(std::string cachekey, int timerid);
    /**
     * @brief 移除延迟更新数据计时器
     * @param cachekey cachekey定义规则为tablename_key 
     * @return 是否成功
    */
    bool remove(std::string cachekey);
    /**
     * @brief 查找延迟更新数据计时器
     * @param cachekey cachekey定义规则为tablename_key 
     * @return 计时器id,找不到返回-1
    */
    int find(std::string cachekey);

private:
    /**
     * @brief 互斥体
    */
    std::mutex cachekey_timerid_mutex_;
    /**
     * @brief cachekey对应计时处理器id的hash表
    */
    std::unordered_map<std::string, int> cachekey_timerids_;
};
}

#endif
