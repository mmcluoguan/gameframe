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
        explicit AcceptReadIoTask(std::weak_ptr<net::AcceptNewFd> newfd);
        ~AcceptReadIoTask() = default;

        /**
         * @brief ������
        */
        void operator()();

    private:
        /**
         * @brief ���տͻ������ݴ�����
        */
        std::weak_ptr<net::AcceptNewFd> newfd_;
    };
}
}

#endif
