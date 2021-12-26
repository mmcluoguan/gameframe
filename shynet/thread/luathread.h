#ifndef SHYNET_THREAD_LUATHREAD_H
#define SHYNET_THREAD_LUATHREAD_H

#include "3rd/kaguya/kaguya.hpp"
#include "shynet/lua/luatask.h"
#include "shynet/thread/thread.h"
#include <condition_variable>
#include <mutex>
#include <queue>

namespace shynet {
namespace thread {
    /**
     * @brief lua线程
    */
    class LuaThread : public Thread {
    public:
        /**
         * @brief 构造
         * @param index 线程在线程池中的索引
        */
        explicit LuaThread(size_t index);
        ~LuaThread() = default;

        /**
         * @brief 线程运行回调
         * @return 0成功 -1失败
        */
        int run() override;
        /**
         * @brief 安全终止线程
         * @return 0成功 -1失败
        */
        int stop() override;

        /**
         * @brief 添加lua任务
         * @param tk lua任务
        */
        void addTask(std::shared_ptr<luatask::LuaTask> tk);

        /**
         * @brief 获取lua状态
         * @return lua状态
        */
        kaguya::State* luaState() const { return luaState_; }

    private:
        /**
         * @brief 互斥体
        */
        std::mutex tasks_mutex_;
        /**
         * @brief 条件变量
        */
        std::condition_variable tasks_condvar_;
        /**
         * @brief lua任务队列
        */
        std::queue<std::shared_ptr<luatask::LuaTask>> tasks_;
        /**
         * @brief 线程结束标识
        */
        bool stop_ = false;
        /**
         * @brief lua状态
        */
        kaguya::State* luaState_ = nullptr;
    };
}
}

#endif
