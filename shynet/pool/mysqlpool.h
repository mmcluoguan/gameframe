#ifndef SHYNET_POOL_MYSQLPOOL_H
#define SHYNET_POOL_MYSQLPOOL_H

#include "3rd/mysqlx/xdevapi.h"
#include "shynet/utils/singleton.h"
#include <condition_variable>
#include <functional>
#include <queue>

namespace shynet {
namespace pool {
    /**
     * @brief  mysql连接池
    */
    class MysqlPool final : public Nocopy {
        friend class utils::Singleton<MysqlPool>;

        /**
         * @brief 构造
         * @param opt mysql连接配置
         * @param capactiy 容量
        */
        MysqlPool(const mysqlx::SessionSettings& opt, size_t capactiy = 1);

    public:
        /**
         * @brief 类型名称
        */
        static constexpr const char* kClassname = "MysqlPool";

        /**
         * @brief 连接会话指针
        */
        using SessionPtr = std::unique_ptr<mysqlx::Session,
            std::function<void(mysqlx::Session*)>>;
        /**
         * @brief 连接会话队列
        */
        using SessionQueue = std::queue<SessionPtr>;

        ~MysqlPool();

        /**
         * @brief 获取连接会话
         * @return 连接会话
        */
        SessionPtr fetch();

        /**
         * @brief 回收连接会话到连接池
         * @param 连接会话 
        */
        void reclaim(mysqlx::Session*);

    private:
        /**
         * @brief 回收函数
        */
        std::function<void(mysqlx::Session*)> fun_;
        /**
         * @brief 互斥体
        */
        std::mutex queue_mutex_;
        /**
         * @brief 连接会话队列
        */
        SessionQueue queue_;
        /**
         * @brief 条件变量
        */
        std::condition_variable cond_var_;
        /**
         * @brief mysql连接配置
        */
        mysqlx::SessionSettings option_;
        /**
         * @brief 容量
        */
        size_t capactiy_ = 1;
        /**
         * @brief 当前建立的连接数
        */
        size_t use_num_ = 0;
        /**
         * @brief 回收连接会话时是否断开与mysql的连接
        */
        bool enable_del_ = false;
    };
}
}

#endif
