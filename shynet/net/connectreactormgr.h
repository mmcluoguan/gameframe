#ifndef SHYNET_NET_CONNECTREACTORMGR_H
#define SHYNET_NET_CONNECTREACTORMGR_H

#include "shynet/net/connectevent.h"
#include "shynet/utils/singleton.h"
#include <unordered_map>

namespace shynet {
namespace net {
    /**
     * @brief ���ӵķ�������ַ������
    */
    class ConnectReactorMgr final : public Nocopy {
        friend class utils::Singleton<ConnectReactorMgr>;
        ConnectReactorMgr() = default;

    public:
        /**
         * @brief ��������
        */
        static constexpr const char* kClassname = "ConnectReactorMgr";
        ~ConnectReactorMgr() = default;

        /**
         * @brief ����µĽ��շ��������ݴ�����
         * @param v �µĽ��շ��������ݴ�����
         * @return �ͻ���id
        */
        int add(std::shared_ptr<ConnectEvent> v);
        /**
         * @brief �Ƴ����շ��������ݴ�����
         * @param k �ͻ���id
         * @return ture�ɹ�,falseʧ��
        */
        bool remove(int k);

        /**
         * @brief ���ҽ��շ��������ݴ�����
         * @param k �ͻ���id
         * @return �ҵ����ؽ��շ��������ݴ�����,����nullptr
        */
        std::shared_ptr<ConnectEvent> find(int k);
        /**
         * @brief ���ҽ��շ��������ݴ�����
         * @param ip ��ַ�ַ���
         * @param port �˿�
         * @return �ҵ����ؽ��շ��������ݴ�����,����nullptr
        */
        std::shared_ptr<ConnectEvent> find(const std::string ip, unsigned short port);

    private:
        /**
         * @brief ֪ͨ���ӷ��������߳����µ���������
         * @param data �ͻ���id�ĵ�ַ
         * @param len �ͻ���id��ַ��С
        */
        void notify(const void* data, size_t len);

    private:
        /**
         * @brief ������
        */
        std::mutex cnt_mutex_;
        /**
         * @brief ���շ��������ݴ�������hash��
        */
        std::unordered_map<int, std::shared_ptr<ConnectEvent>> cnts_;
    };
}
}

#endif
