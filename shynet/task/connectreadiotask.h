#ifndef SHYNET_TASK_CONNECTREADIOTASK_H
#define SHYNET_TASK_CONNECTREADIOTASK_H

#include "shynet/net/connectevent.h"
#include "shynet/thread/thread.h"

namespace shynet {
namespace task {
    /**
     * @brief ���շ�����������������
    */
    class ConnectReadIoTask {
    public:
        /**
         * @brief ����
         * @param cntevent ���շ��������ݴ�����
        */
        explicit ConnectReadIoTask(std::shared_ptr<net::ConnectEvent> cntevent);
        ~ConnectReadIoTask() = default;

        /**
         * @brief ������
        */
        void operator()();

    private:
        /**
         * @brief ���շ��������ݴ�����
        */
        std::shared_ptr<net::ConnectEvent> cntevent_;
    };
}
}

#endif
