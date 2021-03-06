#include "shynet/thread/workthread.h"
#include "shynet/pool/threadpool.h"
#include "shynet/utils/stuff.h"

namespace shynet {
namespace thread {
    WorkThread::WorkThread(size_t index)
        : Thread(ThreadType::GENERAL, index)
    {
    }

    int WorkThread::run()
    {
        try {
            while (stop_ == false) {
                std::function<void()> tk;
                if (tasks_.empty() == false) {
                    std::lock_guard<std::mutex> lock(tasks_mutex_);
                    tk = tasks_.front();
                    tasks_.pop();
                    LOG_TRACE << "pop local queue task";
                } else {
                    pool::ThreadPool& tpool = utils::Singleton<pool::ThreadPool>::instance();
                    std::unique_lock<std::mutex> lock(tpool.tasks_mutex());
                    tpool.tasks_condvar().wait(lock, [&tpool, this] {
                        LOG_TRACE << "start front check  globalsize:" << tpool.tasks().size() << " stop:" << stop_ << " localsize:" << tasks_.size();
                        return !tpool.tasks().empty() || stop_ == true || (!tasks_.empty());
                    });
                    LOG_TRACE << "wake-up after check globalsize:" << tpool.tasks().size() << " stop:" << stop_ << " localsize:" << tasks_.size();
                    if (stop_) {
                        LOG_TRACE << "wake-up due to thread pool release";
                        break;
                    }
                    if (tpool.tasks().empty() == false) {
                        tk = tpool.tasks().front();
                        tpool.tasks().pop();
                        LOG_TRACE << "pop global queue task";
                    } else
                        continue;
                }
                if (tk != nullptr) {
                    try {
                        tk();
                    } catch (const std::exception& err) {
                        utils::stuff::print_exception(err);
                    }
                }
            }
            //清空未完成任务
            while (tasks_.empty() == false) {
                tasks_.front()();
                tasks_.pop();
            }
        } catch (const std::exception& err) {
            utils::stuff::print_exception(err);
        }
        return 0;
    }

    int WorkThread::stop()
    {
        stop_ = true;
        return 0;
    }

    void WorkThread::addTask(std::function<void()>&& tk)
    {
        std::lock_guard<std::mutex> lock(tasks_mutex_);
        tasks_.push(tk);
    }

}
}
