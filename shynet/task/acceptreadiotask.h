#ifndef SHYNET_TASK_ACCEPTREADIOTASK_H
#define SHYNET_TASK_ACCEPTREADIOTASK_H

#include "shynet/net/acceptnewfd.h"
#include "shynet/thread/thread.h"

namespace shynet {
namespace task {
    /**
     * @brief ���տͻ���������������
    */
    class AcceptReadIoTask {
    public:
        /**
         * @brief ����
         * @param newfd ���տͻ������ݴ�����
        */
        explicit AcceptReadIoTask(std::shared_ptr<net::AcceptNewFd> newfd,
            std::unique_ptr<char[]> complete_data, size_t length);
        ~AcceptReadIoTask() = default;

        /**
         * @brief ������
        */
        void operator()();

    private:
        /**
         * @brief ���տͻ������ݴ�����
        */
        std::shared_ptr<net::AcceptNewFd> newfd_;
        std::unique_ptr<char[]> complete_data_;
        size_t complete_data_length_;
    };
}
}

#endif
