#ifndef SHYNET_POOL_THREADPOOL_H
#define SHYNET_POOL_THREADPOOL_H

#include "shynet/task/Task.h"
#include "shynet/thread/AcceptThread.h"
#include "shynet/thread/ConnectThread.h"
#include "shynet/thread/InotifyThread.h"
#include "shynet/thread/ListenThread.h"
#include "shynet/thread/LuaThread.h"
#include "shynet/thread/TimerThread.h"
#include "shynet/thread/WorkThread.h"
#include "shynet/utils/Singleton.h"

namespace shynet {
namespace pool {
    /// <summary>
    /// 线程池
    /// </summary>
    class ThreadPool final : public Nocopy {
        friend class utils::Singleton<ThreadPool>;

        /// <summary>
        /// 初始化线程池
        /// </summary>
        /// <param name="workNum">work线程数量</param>
        /// <param name="acceptNum">接收线程数量</param>
        explicit ThreadPool(size_t workNum = 4, size_t acceptNum = 2);

    public:
        static std::string classname;
        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;
        ~ThreadPool();

        void start();

        /// <summary>
        /// 添加任务到线程池公共的任务队列中
        /// </summary>
        /// <param name="tk"></param>
        void append(std::shared_ptr<task::Task> tk);
        /// <summary>
        /// 添加任务到线程池中指定工作线程
        /// 注意,此方法目的是让任务在特定的线程按添加的顺序执行
        /// </summary>
        /// <param name="tk"></param>
        /// <param name="tag">
        /// 方法内按照(tag%普通工作线程数量)求得线程指定位置
        /// </param>
        void appendWork(std::shared_ptr<task::Task> tk, size_t tag);

        std::mutex& tasks_mutex()
        {
            return tasks_mutex_;
        }

        std::condition_variable& tasks_condvar()
        {
            return tasks_condvar_;
        }

        std::queue<std::shared_ptr<task::Task>>& tasks()
        {
            return tasks_;
        }

        std::weak_ptr<thread::ListenThread> listernTh() const
        {
            return listernTh_;
        }

        std::vector<std::weak_ptr<thread::AcceptThread>> acceptThs() const
        {
            return acceptThs_;
        }

        std::weak_ptr<thread::TimerThread> timerTh() const
        {
            return timerTh_;
        }

        std::weak_ptr<thread::ConnectThread> connectTh() const
        {
            return connectTh_;
        }

        std::weak_ptr<thread::LuaThread> luaTh() const
        {
            return luaTh_;
        }

        std::weak_ptr<thread::InotifyThread> notifyTh() const
        {
            return notifyTh_;
        }

    private:
        size_t acceptNum_ = 2;
        size_t workNum_ = 4;
        /// <summary>
        /// 线程列表
        /// </summary>
        std::vector<std::shared_ptr<thread::Thread>> tifs_;

        std::mutex tasks_mutex_;
        std::condition_variable tasks_condvar_;
        /// <summary>
        /// 公共的任务队列中等待执行的任务
        /// </summary>
        std::queue<std::shared_ptr<task::Task>> tasks_;

        /*
			* 监听线程
			*/
        std::weak_ptr<thread::ListenThread> listernTh_;
        /*
			* 接收线程列表
			*/
        std::vector<std::weak_ptr<thread::AcceptThread>> acceptThs_;
        /*
			* 工作线程列表
			*/
        std::vector<std::weak_ptr<thread::WorkThread>> workThs_;
        /*
			* 计时器线程
			*/
        std::weak_ptr<thread::TimerThread> timerTh_;
        /*
			* 连接线程
			*/
        std::weak_ptr<thread::ConnectThread> connectTh_;
        /*
			* lua线程
			*/
        std::weak_ptr<thread::LuaThread> luaTh_;
        /*
			* 目录监控线程
			*/
        std::weak_ptr<thread::InotifyThread> notifyTh_;
    };
}
}

#endif
