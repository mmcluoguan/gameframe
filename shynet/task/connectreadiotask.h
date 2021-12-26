#ifndef SHYNET_TASK_CONNECTREADIOTASK_H
#define SHYNET_TASK_CONNECTREADIOTASK_H

#include "shynet/net/connectevent.h"
#include "shynet/thread/thread.h"

namespace shynet {
namespace task {
    /**
     * @brief 接收服务器数据任务处理器
    */
    class ConnectReadIoTask {
    public:
        /**
         * @brief 构造
         * @param cntevent 接收服务器数据处理器
        */
        explicit ConnectReadIoTask(std::shared_ptr<net::ConnectEvent> cntevent);
        ~ConnectReadIoTask() = default;

        /**
         * @brief 任务处理
        */
        void operator()();

    private:
        /**
         * @brief 接收服务器数据处理器
        */
        std::shared_ptr<net::ConnectEvent> cntevent_;
    };
}
}

#endif
