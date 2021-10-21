#ifndef SHYNET_THREAD_WORKTHREAD_H
#define SHYNET_THREAD_WORKTHREAD_H

#include "shynet/task/Task.h"
#include "shynet/thread/Thread.h"
#include <mutex>
#include <queue>

namespace shynet {
namespace thread {
    class WorkThread : public Thread {
    public:
        explicit WorkThread(size_t index);
        ~WorkThread();

        int run() override;
        int stop() override;

        size_t addTask(std::shared_ptr<task::Task> tk);

    private:
        std::mutex tasks_mutex_;
        std::queue<std::shared_ptr<task::Task>> tasks_;
        bool stop_ = false;
    };
}
}

#endif
