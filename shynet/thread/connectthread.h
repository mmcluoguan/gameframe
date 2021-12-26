#ifndef SHYNET_THREAD_CONNECTTHREAD_H
#define SHYNET_THREAD_CONNECTTHREAD_H

#include "shynet/events/eventbuffer.h"
#include "shynet/thread/thread.h"

namespace shynet {
namespace thread {
    /**
     * @brief ���ӷ������߳�
    */
    class ConnectThread : public Thread {
    public:
        /**
         * @brief ����
         * @param index �߳����̳߳��е�����
        */
        explicit ConnectThread(size_t index);
        ~ConnectThread() = default;

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
         * @brief ֪ͨ���ӷ������߳̽����µ�����
         * @param data �ͻ���id�ĵ�ַ
         * @param len �ͻ���id��ַ��С
        */
        int notify(const void* data, size_t len) const;

        /**
         * @brief ����֪ͨ������Ϣ
         * @param bev ֪ͨ������Ϣ
        */
        void process(struct bufferevent* bev);

    private:
        /**
         * @brief ����֪ͨ�Ĺܵ�
        */
        std::shared_ptr<events::EventBuffer> pair_[2] = { 0 };
        /**
         * @brief ��Ӧ��
        */
        std::shared_ptr<events::EventBase> base_ = nullptr;
    };
}
}

#endif
