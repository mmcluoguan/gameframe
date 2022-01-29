#ifndef SHYNET_NET_TIMERREACTORMGR_H
#define SHYNET_NET_TIMERREACTORMGR_H

#include "shynet/net/timerevent.h"
#include "shynet/utils/singleton.h"
#include <unordered_map>

namespace shynet {
namespace net {
    /**
     * @brief ��ʱ��������
    */
    class TimerReactorMgr final : public Nocopy {
        friend class utils::Singleton<TimerReactorMgr>;

        /**
         * @brief ����
        */
        TimerReactorMgr() = default;

    public:
        /**
         * @brief ��������
        */
        static constexpr const char* kClassname = "TimerReactorMgr";
        ~TimerReactorMgr() = default;

        /**
         * @brief ����µļ�ʱ��������
         * @param v �µļ�ʱ��������
         * @return ��ʱ��id
        */
        int add(std::shared_ptr<TimerEvent> v);
        /**
         * @brief �Ƴ���ʱ��������
         * @param k ��ʱ��id
         * @return ture�ɹ�,falseʧ��
        */
        bool remove(int k);
        /**
         * @brief ���Ҽ�ʱ��������
         * @param k ��ʱ��id
         * @return �ҵ����ؼ�ʱ��������,����nullptr
        */
        std::shared_ptr<TimerEvent> find(int k);

    private:
        /**
         * @brief ֪ͨ��ʱ���߳����µļ�ʱ������
         * @param timerid
        */
        void notify(int timerid);

    private:
        /**
         * @brief ������
        */
        std::mutex times_mutex_;
        /**
         * @brief ��ʱ����������hash��
        */
        std::unordered_map<int, std::shared_ptr<TimerEvent>> times_;
    };
}
}

#endif
