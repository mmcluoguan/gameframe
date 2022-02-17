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
        explicit ConnectReadIoTask(std::shared_ptr<net::ConnectEvent> cntevent, std::unique_ptr<char[]> complete_data, size_t length);
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
        std::unique_ptr<char[]> complete_data_;
        size_t complete_data_length_;
    };
}
}

#endif
