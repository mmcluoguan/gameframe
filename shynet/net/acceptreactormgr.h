#ifndef SHYNET_NET_ACCEPTREACTORMGR_H
#define SHYNET_NET_ACCEPTREACTORMGR_H

#include "shynet/thread/acceptthread.h"
#include "shynet/utils/singleton.h"

namespace shynet {
namespace net {
    /**
     * @brief 接收客户端连接线程管理器
    */
    class AcceptReactorMgr final : public Nocopy {
        friend class utils::Singleton<AcceptReactorMgr>;

        /**
         * @brief 构造
        */
        AcceptReactorMgr() = default;

    public:
        /**
         * @brief 类型名称
        */
        static constexpr const char* kClassname = "AcceptReactorMgr";
        ~AcceptReactorMgr() = default;

        /**
         * @brief 通知服务器接收客户端数据线程接收新的连接
         * @param serverid
        */
        void notify(int serverid);

    private:
        /**
         * @brief 查找最优连接线程
         * @return 最优连接线程
        */
        std::shared_ptr<thread::AcceptThread> find_rtk() const;
    };
}
}

#endif
