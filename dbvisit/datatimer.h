#ifndef DBVISIT_DATATIMER_H
#define DBVISIT_DATATIMER_H

#include "shynet/net/timerevent.h"
#include <unordered_map>

namespace dbvisit {
/**
 * @brief 延迟更新数据计时处理器
*/
class DataTimer : public net::TimerEvent {
public:
    /**
     * @brief 构造
     * @param cachekey cachekey定义规则为tablename_key 
     * @param val 超时相对时间值
    */
    DataTimer(const std::string& cachekey, const struct timeval val);
    ~DataTimer() = default;

    /**
    * @brief 计时器超时后,在工作线程中处理超时回调
    */
    void timeout() override;
    /**
     * @brief 更新缓存的数据
     * @param data 新的hash数据 
    */
    void modify_cache_fields(const std::unordered_map<std::string, std::string>& data);

private:
    /**
     * @brief cachekey定义规则为tablename_key 
    */
    std::string cachekey_;
    /**
     * @brief key对应value的hash表
    */
    std::unordered_map<std::string, std::string> fields_;
};
}

#endif
