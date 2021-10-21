#ifndef SHYNET_POOL_MYSQLPOOL_H
#define SHYNET_POOL_MYSQLPOOL_H

#include "shynet/utils/Singleton.h"
#include <condition_variable>
#include <functional>
#include <mysqlx/xdevapi.h>
#include <queue>

namespace shynet {
namespace pool {
    /*
		* mysql连接池
		*/
    class MysqlPool final : public Nocopy {
        friend class utils::Singleton<MysqlPool>;

        MysqlPool(const mysqlx::SessionSettings& opt, size_t capactiy = 1);

    public:
        using SessionPtr = std::unique_ptr<mysqlx::Session,
            std::function<void(mysqlx::Session*)>>;
        using SessionQueue = std::queue<SessionPtr>;

        static constexpr const char* classname = "MysqlPool";
        ~MysqlPool();

        /*
			* 取出一个会话
			*/
        SessionPtr fetch();

        /*
			* 回收会话到连接池
			*/
        void reclaim(mysqlx::Session*);

    private:
        std::function<void(mysqlx::Session*)> fun_;
        std::mutex queue_mutex_;
        SessionQueue queue_;
        std::condition_variable cond_var_;
        mysqlx::SessionSettings option_;
        size_t capactiy_ = 1;
        size_t use_num_ = 0;
        bool enable_del_ = false;
    };
}
}

#endif
