#include "shynet/pool/MysqlPool.h"

namespace shynet {
namespace pool {
    MysqlPool::MysqlPool(const mysqlx::SessionSettings& opt, size_t capactiy)
        : option_(opt)
    {
        capactiy_ = capactiy;
        fun_ = std::bind(&MysqlPool::reclaim, this, std::placeholders::_1);
    }
    MysqlPool::~MysqlPool()
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        enable_del_ = true;
    }
    MysqlPool::SessionPtr MysqlPool::fetch()
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        if (queue_.empty()) {
            if (use_num_ == capactiy_) {
                cond_var_.wait(lock, [this] { return !this->queue_.empty(); });
            } else {
                SessionPtr session(new mysqlx::Session(option_), fun_);
                ++use_num_;
                return session;
            }
        }
        ++use_num_;
        SessionPtr sptr = std::move(queue_.front());
        queue_.pop();
        try {
            sptr->sql("select 1").execute();
        } catch (const mysqlx::Error& err) {
            sptr.release();
            SessionPtr session(new mysqlx::Session(option_), fun_);
            return session;
        }
        return sptr;
    }
    void MysqlPool::reclaim(mysqlx::Session* ses)
    {
        if (enable_del_) {
            ses->close();
            delete ses;
            return;
        }
        SessionPtr ptr(ses, fun_);
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            queue_.push(std::move(ptr));
            --use_num_;
        }
        cond_var_.notify_one();
    }
} // namespace pool
} // namespace shynet
