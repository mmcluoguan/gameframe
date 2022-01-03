#ifndef SHYNET_THREAD_ACCEPTTHREAD_H
#define SHYNET_THREAD_ACCEPTTHREAD_H

#include "shynet/events/eventbuffer.h"
#include "shynet/thread/thread.h"

namespace shynet {
namespace thread {
    /**
     * @brief ���������տͻ��������߳�
    */
    class AcceptThread : public Thread {
    public:
        /**
         * @brief ����
         * @param index �߳����̳߳��е����� 
        */
        explicit AcceptThread(size_t index);
        ~AcceptThread() = default;

        /**
         * @brief ��ȡ�ۼƽ��տͻ�����������
         * @return �ۼƽ��տͻ�����������
        */
        int event_tot() const { return eventTot_; }

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
         * @brief ֪ͨ���������տͻ��������߳̽����µ�����
         * @param data ����Ϊ ListenEvent*
         * @param len ���ݳ���
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
        /**
         * @brief �ۼƽ��տͻ�����������
        */
        int eventTot_ = 0;
    };
}
}

#endif
