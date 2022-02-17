#ifndef SHYNET_TASK_ACCEPTREADIOTASK_H
#define SHYNET_TASK_ACCEPTREADIOTASK_H

#include "shynet/net/acceptnewfd.h"
#include "shynet/thread/thread.h"

namespace shynet {
namespace task {
    /**
     * @brief 接收客户端数据任务处理器
    */
    class AcceptReadIoTask {
    public:
        /**
         * @brief 构造
         * @param newfd 接收客户端数据处理器
        */
        explicit AcceptReadIoTask(std::shared_ptr<net::AcceptNewFd> newfd,
            std::unique_ptr<char[]> complete_data, size_t length);
        ~AcceptReadIoTask() = default;

        /**
         * @brief 任务处理
        */
        void operator()();

    private:
        /**
         * @brief 接收客户端数据处理器
        */
        std::shared_ptr<net::AcceptNewFd> newfd_;
        std::unique_ptr<char[]> complete_data_;
        size_t complete_data_length_;
    };
}
}

#endif
