#ifndef SHYNET_THREAD_THREAD_H
#define SHYNET_THREAD_THREAD_H

#include <shynet/basic.h>
#include <thread>

namespace shynet {
namespace thread {
    /**
     * @brief 线程信息
    */
    class Thread : public Nocopy {
    public:
        /**
         * @brief 线程类型
        */
        enum class ThreadType {
            /**
             * @brief 工作线程
            */
            GENERAL,
            /**
             * @brief 计时器线程
            */
            TIMER,
            /**
             * @brief 服务器接收客户端数据
            */
            ACCEPT,
            /**
             * @brief 连接服务器线程
            */
            CONNECT,
            /**
             * @brief 监听服务器地址
            */
            LISTEN,
            /**
             * @brief lua线程
            */
            LUA,
            /**
             * @brief 目录监控线程
            */
            INOTIFY,
            /**
             * @brief udp逻辑线程
            */
            UDP

        };

        /**
         * @brief 构造
         * @param t 线程类型
         * @param index 线程池中的索引
        */
        Thread(ThreadType t, size_t index);
        ~Thread() = default;

        /**
         * @brief 获取线程类型
         * @return 线程类型
        */
        ThreadType type() const { return type_; }
        /**
         * @brief 设置线程类型
         * @param v 线程类型
        */
        void set_type(ThreadType v) { type_ = v; }

        /**
         * @brief 获取线程在线程池中的索引
         * @return 线程在线程池中的索引
        */
        size_t index() const { return index_; }
        /**
         * @brief 设置线程在线程池中的索引
         * @param v 线程在线程池中的索引
        */
        void set_index(size_t v) { index_ = v; }

        /**
         * @brief 获取原生线程信息
         * @return 原生线程信息
        */
        std::shared_ptr<std::thread> thread() const { return thread_; }
        /**
         * @brief 启动线程
         * @return 原生线程信息
        */
        std::shared_ptr<std::thread> start();
        /**
         * @brief 线程运行回调
         * @return 0成功 -1失败
        */
        virtual int run() = 0;
        /**
         * @brief 安全终止线程
         * @return 0成功 -1失败
        */
        virtual int stop() = 0;

    private:
        /**
         * @brief 线程类型
        */
        ThreadType type_ = ThreadType::GENERAL;
        /**
         * @brief 原生线程信息
        */
        std::shared_ptr<std::thread> thread_ = nullptr;
        /**
         * @brief 线程在线程池中的索引
        */
        size_t index_ = -1;
    };
}
}

#endif
