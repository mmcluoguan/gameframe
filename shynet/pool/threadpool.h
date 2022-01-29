#ifndef SHYNET_POOL_THREADPOOL_H
#define SHYNET_POOL_THREADPOOL_H

#include "shynet/thread/acceptthread.h"
#include "shynet/thread/connectthread.h"
#include "shynet/thread/inotifythread.h"
#include "shynet/thread/listenthread.h"
#include "shynet/thread/luathread.h"
#include "shynet/thread/timerthread.h"
#include "shynet/thread/udpthread.h"
#include "shynet/thread/workthread.h"
#include "shynet/utils/singleton.h"
#include <functional>
#include <future>

namespace shynet {
namespace pool {
    /**
     * @brief 线程池
    */
    class ThreadPool final : public Nocopy {
        friend class utils::Singleton<ThreadPool>;

        /**
         * @brief 构造
         * @param workNum work线程数量
         * @param acceptNum 服务器接收新连接线程数量
        */
        explicit ThreadPool(size_t workNum = 4, size_t acceptNum = 1);

    public:
        /**
         * @brief 类型名称
        */
        static constexpr const char* kClassname = "ThreadPool";

        ~ThreadPool();

        /**
         * @brief 启动
        */
        void start();

        /**
         * @brief 添加任务到线程池公共的任务队列中
         *
         * @tparam F 可调用对象类型
         * @tparam A 参数列表类型
         * @param task 可调用对象
         * @param args 参数
         * @return 返回future
         */
        template <typename F, typename... A, typename R = std::invoke_result_t<std::decay_t<F>, std::decay_t<A>...>>
        std::future<R> append(F&& task, A&&... args)
        {
            std::shared_ptr<std::promise<R>> task_promise = std::make_shared<std::promise<R>>();
            std::future<R> future = task_promise->get_future();
            auto tk = [task, args..., task_promise]() {
                if constexpr (std::is_void_v<R>) {
                    task(args...);
                } else {
                    task_promise->set_value(task(args...));
                }
            };
            std::lock_guard<std::mutex> lock(tasks_mutex_);
            tasks_.push(tk);
            LOG_TRACE << "append notify_one";
            tasks_condvar_.notify_one();
            return future;
        }

        /**
         * @brief 添加任务到线程池公共的任务队列中
         * shared_ptr 版本
         *
         * @tparam F 可调用对象类型
         * @tparam A 参数列表类型
         * @param task 可调用对象
         * @param args 参数
         * @return 返回future
         */
        template <typename F, typename... A, typename R = std::invoke_result_t<std::decay_t<F>, std::decay_t<A>...>>
        std::future<R> append(std::shared_ptr<F>& task, A&&... args)
        {
            std::shared_ptr<std::promise<R>> task_promise = std::make_shared<std::promise<R>>();
            std::future<R> future = task_promise->get_future();
            auto tk = [task, args..., task_promise]() {
                if constexpr (std::is_void_v<R>) {
                    (*task)(args...);
                } else {
                    task_promise->set_value((*task)(args...));
                }
            };
            std::lock_guard<std::mutex> lock(tasks_mutex_);
            tasks_.push(tk);
            LOG_TRACE << "append notify_one";
            tasks_condvar_.notify_one();
            return future;
        }

        /**
         * @brief 添加任务到线程池中指定工作线程
         *
         * @tparam F 可调用对象类型
         * @tparam A 参数列表类型
         * @param tag 方法内按照(tag%普通工作线程数量)求得线程指定位置
         * @param task 可调用对象
         * @param args 参数
         * @return 返回future
         */
        template <typename F, typename... A, typename R = std::invoke_result_t<std::decay_t<F>, std::decay_t<A>...>>
        std::future<R> appendwork(size_t tag, F&& task, A&&... args)
        {
            std::shared_ptr<std::promise<R>> task_promise = std::make_shared<std::promise<R>>();
            std::future<R> future = task_promise->get_future();
            std::shared_ptr<thread::WorkThread> target = workThs_[tag % workThs_.size()].lock();
            if (target) {
                auto tk = [task, args..., task_promise]() {
                    if constexpr (std::is_void_v<R>) {
                        task(args...);
                    } else {
                        task_promise->set_value(task(args...));
                    }
                };
                target->addTask(tk);
                LOG_TRACE << "appendwork notify_all";
                tasks_condvar_.notify_all();
            } else {
                if constexpr (std::is_void_v<R>) {
                    task(args...);
                } else {
                    task_promise->set_value(task(args...));
                }
            }
            return future;
        }

        /**
         * @brief 添加任务到线程池中指定工作线程
         * shared_ptr 版本
         *
         * @tparam F 可调用对象类型
         * @tparam A 参数列表类型
         * @param tag 方法内按照(tag%普通工作线程数量)求得线程指定位置
         * @param task 可调用对象
         * @param args 参数
         * @return 返回future
         */
        template <typename F, typename... A, typename R = std::invoke_result_t<std::decay_t<F>, std::decay_t<A>...>>
        std::future<R> appendwork(size_t tag, std::shared_ptr<F>& task, A&&... args)
        {
            std::shared_ptr<std::promise<R>> task_promise = std::make_shared<std::promise<R>>();
            std::future<R> future = task_promise->get_future();
            std::shared_ptr<thread::WorkThread> target = workThs_[tag % workThs_.size()].lock();
            if (target) {
                auto tk = [task, args..., task_promise]() {
                    if constexpr (std::is_void_v<R>) {
                        (*task)(args...);
                    } else {
                        task_promise->set_value((*task)(args...));
                    }
                };
                target->addTask(tk);
                LOG_TRACE << "appendwork notify_all";
                tasks_condvar_.notify_all();
            } else {
                if constexpr (std::is_void_v<R>) {
                    (*task)(args...);
                } else {
                    task_promise->set_value((*task)(args...));
                }
            }
            return future;
        }

        /**
         * @brief 获取任务互斥体
         * @return 任务互斥体
        */
        std::mutex& tasks_mutex()
        {
            return tasks_mutex_;
        }

        /**
         * @brief 获取任务条件变量
         * @return 任务条件变量
        */
        std::condition_variable& tasks_condvar()
        {
            return tasks_condvar_;
        }

        /**
         * @brief 获取全局任务队列
         * @return 全局任务队列
        */
        std::queue<std::function<void()>>& tasks()
        {
            return tasks_;
        }

        /**
         * @brief 获取监听服务器地址线程
         * @return 监听服务器地址线程
        */
        std::weak_ptr<thread::ListenThread> listernTh() const
        {
            return listernTh_;
        }

        /**
         * @brief 迭代服务器接收客户端数据线程
         * @param cb 迭代回调
        */
        void foreach_acceptThs(std::function<void(std::weak_ptr<thread::AcceptThread>)> cb) const
        {
            for (auto& it : acceptThs_) {
                cb(it);
            }
        }

        /**
         * @brief 获取计时器线程
         * @return 计时器线程
        */
        std::weak_ptr<thread::TimerThread> timerTh() const
        {
            return timerTh_;
        }

        /**
         * @brief 获取连接服务器线程
         * @return 连接服务器线程
        */
        std::weak_ptr<thread::ConnectThread> connectTh() const
        {
            return connectTh_;
        }

        /**
         * @brief 获取lua线程
         * @return lua线程
        */
        std::weak_ptr<thread::LuaThread> luaTh() const
        {
            return luaTh_;
        }

        /**
         * @brief 获取文件监控线程
         * @return 文件监控线程
        */
        std::weak_ptr<thread::InotifyThread> notifyTh() const
        {
            return notifyTh_;
        }

        /**
         * @brief 获取udp逻辑线程
         * @return udp逻辑线程
        */
        std::weak_ptr<thread::UdpThread> udpTh() const
        {
            return udpTh_;
        }

    private:
        /**
         * @brief 服务器接收新连接线程数量
        */
        size_t acceptNum_ = 2;
        /**
         * @brief work线程数量
        */
        size_t workNum_ = 4;
        /**
         * @brief 管理的所有线程向量
        */
        std::vector<std::shared_ptr<thread::Thread>> tifs_;
        /**
         * @brief 任务互斥体
        */
        std::mutex tasks_mutex_;
        /**
         * @brief 任务条件变量
        */
        std::condition_variable tasks_condvar_;
        /**
         * @brief 全局任务队列
        */
        std::queue<std::function<void()>> tasks_;

        /*
		* 监听服务器地址线程
		*/
        std::weak_ptr<thread::ListenThread> listernTh_;
        /*
		* 服务器接收客户端数据线程
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
		* 连接服务器线程
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
        /**
         * @brief udp逻辑线程
        */
        std::weak_ptr<thread::UdpThread> udpTh_;
    };
}
}

#endif
