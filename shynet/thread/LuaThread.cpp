#include "shynet/thread/LuaThread.h"
#include "shynet/lua/LuaEngine.h"
#include "shynet/utils/Logger.h"
#include "shynet/utils/Stuff.h"

namespace shynet {
extern pthread_barrier_t g_barrier;

namespace thread {
    LuaThread::LuaThread(size_t index)
        : Thread(ThreadType::LUA, index)
    {
    }

    LuaThread::~LuaThread()
    {
    }

    int LuaThread::run()
    {
        try {
            kaguya::State state;
            luaState_ = &state;
            utils::Singleton<lua::LuaEngine>::instance().init(state);
            pthread_barrier_wait(&g_barrier);
            while (stop_ == false) {
                std::shared_ptr<task::Task> tk;
                {
                    std::unique_lock<std::mutex> lock(tasks_mutex_);
                    tasks_condvar_.wait(lock, [this] {
                        LOG_TRACE << "start front check stop:" << stop_ << " lua_localsize : " << tasks_.size();
                        return !tasks_.empty() || stop_ == true;
                    });
                    LOG_TRACE << "wake-up after check stop:" << stop_ << " lua_localsize:" << tasks_.size();
                    if (stop_) {
                        LOG_TRACE << "wake-up due to thread pool release";
                        break;
                    }
                    if (tasks_.empty() == false) {
                        tk = tasks_.front();
                        tasks_.pop();
                        LOG_TRACE << "pop lua local queue task";
                    } else
                        continue;
                }
                if (tk != nullptr) {
                    try {
                        int ret = tk->run(this);
                        if (ret < 0) {
                            LOG_WARN << "thread[" << index() << "] 线程异常退出";
                            return 0;
                        }
                    } catch (const std::exception& err) {
                        utils::Stuff::print_exception(err);
                    }
                }
            }
            //清空未完成任务
            while (tasks_.empty() == false) {
                tasks_.front()->run(this);
                tasks_.pop();
            }
            luaState_ = nullptr;
        } catch (const std::exception& err) {
            utils::Stuff::print_exception(err);
        }
        return 0;
    }

    int LuaThread::stop()
    {
        stop_ = true;
        tasks_condvar_.notify_one();
        return 0;
    }

    size_t LuaThread::addTask(std::shared_ptr<task::Task> tk)
    {
        std::lock_guard<std::mutex> lock(tasks_mutex_);
        tasks_.push(tk);
        tasks_condvar_.notify_one();
        return tasks_.size();
    }

}
}
