#ifndef LOG_LOGTIMER_H
#define LOG_LOGTIMER_H

#include "frmpub/protocc/log.pb.h"
#include "shynet/net/timerevent.h"
#include "shynet/utils/hash.h"
#include <list>

namespace logs {

/**
 * @brief 写日志计时处理器
*/
class LogTimer : public net::TimerEvent {
public:
    /**
     * @brief 构造
     * @param val 超时相对时间值
    */
    LogTimer(const struct timeval val);
    ~LogTimer() = default;
    /**
    * @brief 计时器超时后,在工作线程中处理超时回调
    */
    void timeout() override;

    /**
     * @brief 添加日志信息
     * @param name 名字
     * @param data 信息
    */
    void add(const frmpub::protocc::writelog_to_log_c& data);

private:
    struct LogKey {
        std::string dirname;
        std::string logname;

        bool operator==(const LogKey& p) const
        {
            return p.dirname == dirname && p.logname == logname;
        }
    };

    struct LogKeyHash {
    public:
        size_t operator()(const LogKey& p) const
        {
            return shynet::utils::hash_val(p.dirname, p.logname);
        }
    };

    /**
     * @brief 互斥体
    */
    std::mutex data_mut_;
    /**
     * @brief 日志数据
    */
    std::unordered_map<LogKey, std::list<frmpub::protocc::writelog_to_log_c>, LogKeyHash> data_;
};
}

#endif
