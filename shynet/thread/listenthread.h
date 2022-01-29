#ifndef SHYNET_THREAD_LISTENTHREAD_H
#define SHYNET_THREAD_LISTENTHREAD_H

#include "shynet/events/eventbuffer.h"
#include "shynet/thread/thread.h"

namespace shynet {
namespace thread {
    /**
     * @brief �����ķ�������ַ�߳�
    */
    class ListenThread : public Thread {
    public:
        /**
         * @brief ����
         * @param index �߳����̳߳��е�����
        */
        explicit ListenThread(size_t index);
        ~ListenThread() = default;

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
         * @brief ֪ͨ�����ķ�������ַ�߳����µķ�������ַ��Ҫ����
         * @param serverid
        */
        int notify(int serverid) const;

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
