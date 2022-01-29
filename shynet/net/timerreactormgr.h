#ifndef SHYNET_NET_TIMERREACTORMGR_H
#define SHYNET_NET_TIMERREACTORMGR_H

#include "shynet/net/timerevent.h"
#include "shynet/utils/singleton.h"
#include <unordered_map>

namespace shynet {
namespace net {
    /**
     * @brief 计时器管理器
    */
    class TimerReactorMgr final : public Nocopy {
        friend class utils::Singleton<TimerReactorMgr>;

        /**
         * @brief 构造
        */
        TimerReactorMgr() = default;

    public:
        /**
         * @brief 类型名称
        */
        static constexpr const char* kClassname = "TimerReactorMgr";
        ~TimerReactorMgr() = default;

        /**
         * @brief 添加新的计时器处理器
         * @param v 新的计时器处理器
         * @return 计时器id
        */
        int add(std::shared_ptr<TimerEvent> v);
        /**
         * @brief 移除计时器处理器
         * @param k 计时器id
         * @return ture成功,false失败
        */
        bool remove(int k);
        /**
         * @brief 查找计时器处理器
         * @param k 计时器id
         * @return 找到返回计时器处理器,否则nullptr
        */
        std::shared_ptr<TimerEvent> find(int k);

    private:
        /**
         * @brief 通知计时器线程有新的计时器加入
         * @param timerid
        */
        void notify(int timerid);

    private:
        /**
         * @brief 互斥体
        */
        std::mutex times_mutex_;
        /**
         * @brief 计时器处理器的hash表
        */
        std::unordered_map<int, std::shared_ptr<TimerEvent>> times_;
    };
}
}

#endif
