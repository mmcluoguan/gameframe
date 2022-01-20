#ifndef FRMPUB_LUATIMERMGR_H
#define FRMPUB_LUATIMERMGR_H

#include "shynet/lua/luatask.h"
#include "shynet/net/timerevent.h"
#include "shynet/utils/singleton.h"
#include <unordered_set>

namespace frmpub {
/**
 * @brief lua计时器管理器
*/
class LuaTimerMgr {
    /**
     * @brief lua计时器任务
    */
    class LuaTimerTask : public shynet::luatask::LuaTask {
    public:
        /**
         * @brief 构造
         * @param timerid 计时器id 
        */
        LuaTimerTask(int timerid, std::string funname = "");
        ~LuaTimerTask() = default;
        /**
        * @brief 运行lua任务
        * @param tif 线程信息
        * @return 0成功,-1失败,返回失败lua线程会结束
        */
        int run(thread::Thread* tif) override;

    private:
        /**
         * @brief 计时器id
        */
        int timerid_;
        /**
         * @brief lua计时器调用lua函数名
        */
        std::string funname_;
    };

    /**
     * @brief lua计时处理器
    */
    class LuaTimer : public net::TimerEvent {
    public:
        /**
         * @brief 构造
         * @param val 超时相对时间值
         * @param repeat true为重复执行,false为只执行1次
        */
        LuaTimer(const struct timeval val, bool repeat, std::string funname = "");
        ~LuaTimer() = default;

        /**
         * @brief 计时器超时后,在工作线程中处理超时回调
        */
        void timeout() override;

    private:
        /**
         * @brief lua计时器调用lua函数名
        */
        std::string funname_;
    };

    friend class shynet::utils::Singleton<LuaTimerMgr>;
    /**
     * @brief 构造
    */
    LuaTimerMgr() = default;

public:
    /**
     * @brief 类型名称
    */
    static constexpr const char* kClassname = "LuaTimerMgr";

    /**
     * @brief 添加新的计时处理器
     * @param val 间隔
     * @param repeat 是否重复
     * @param funname lua计时器调用lua函数名
     * @return 计时器id
    */
    int add(const struct timeval val, bool repeat = true, std::string funname = "");
    /**
     * @brief 移除计时处理器
     * @param timerid 计时器id
    */
    void remove(int timerid);

private:
    /**
     * @brief 计时处理器集合
    */
    std::unordered_set<int> timerids_;
};
}

#endif
