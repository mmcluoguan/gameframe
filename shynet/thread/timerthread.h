#ifndef SHYNET_THREAD_TIMERTHREAD_H
#define SHYNET_THREAD_TIMERTHREAD_H

#include "shynet/events/eventbuffer.h"
#include "shynet/thread/thread.h"

namespace shynet {
namespace thread {
    /**
     * @brief ��ʱ���߳�
    */
    class TimerThread : public Thread {
    public:
        /**
         * @brief ����
         * @param index �߳����̳߳��е�����
        */
        explicit TimerThread(size_t index);
        ~TimerThread() = default;

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
         * @brief ֪ͨ��ʱ���߳����µļ�ʱ������
         * @param data ��ʱ��id�ĵ�ַ
         * @param len ��ʱ��id�ĵ�ַ��С
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
