#ifndef SHYNET_THREAD_LUATHREAD_H
#define SHYNET_THREAD_LUATHREAD_H

#include "shynet/task/Task.h"
#include "shynet/thread/Thread.h"
#include <condition_variable>
#include <kaguya/kaguya.hpp>
#include <mutex>
#include <queue>

namespace shynet {
namespace thread {
    class LuaThread : public Thread {
    public:
        explicit LuaThread(size_t index);
        ~LuaThread();

        int run() override;
        int stop() override;

        size_t addTask(std::shared_ptr<task::Task> tk);

        kaguya::State* luaState() const
        {
            return luaState_;
        }

    private:
        std::mutex tasks_mutex_;
        std::condition_variable tasks_condvar_;
        std::queue<std::shared_ptr<task::Task>> tasks_;
        bool stop_ = false;
        kaguya::State* luaState_ = nullptr;
    };
}
}

#endif
