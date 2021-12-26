#ifndef SHYNET_THREAD_WORKTHREAD_H
#define SHYNET_THREAD_WORKTHREAD_H

#include "shynet/thread/thread.h"
#include <mutex>
#include <queue>

namespace shynet {
namespace thread {
    /**
     * @brief work�߳�
    */
    class WorkThread : public Thread {
    public:
        /**
         * @brief ����
         * @param index �߳����̳߳��е�����
        */
        explicit WorkThread(size_t index);
        ~WorkThread() = default;

        /**
         * @brief �߳����лص�
         * @return 0�ɹ� -1ʧ��
        */
        int run() override;
        /**
         * @brief ��ȫ��ֹ�߳�
         * @return 0�ɹ� -1ʧ��
        */
        int stop() override;

        /**
         * @brief ��ӹ�������
         * @param tk ��������
        */
        void addTask(std::function<void()>&& tk);

    private:
        /**
         * @brief ������
        */
        std::mutex tasks_mutex_;
        /**
         * @brief �����������
        */
        std::queue<std::function<void()>> tasks_;
        /**
         * @brief �߳̽�����ʶ
        */
        bool stop_ = false;
    };
}
}

#endif
