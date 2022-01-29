#ifndef SHYNET_NET_ACCEPTREACTORMGR_H
#define SHYNET_NET_ACCEPTREACTORMGR_H

#include "shynet/thread/acceptthread.h"
#include "shynet/utils/singleton.h"

namespace shynet {
namespace net {
    /**
     * @brief ���տͻ��������̹߳�����
    */
    class AcceptReactorMgr final : public Nocopy {
        friend class utils::Singleton<AcceptReactorMgr>;

        /**
         * @brief ����
        */
        AcceptReactorMgr() = default;

    public:
        /**
         * @brief ��������
        */
        static constexpr const char* kClassname = "AcceptReactorMgr";
        ~AcceptReactorMgr() = default;

        /**
         * @brief ֪ͨ���������տͻ��������߳̽����µ�����
         * @param serverid
        */
        void notify(int serverid);

    private:
        /**
         * @brief �������������߳�
         * @return ���������߳�
        */
        std::shared_ptr<thread::AcceptThread> find_rtk() const;
    };
}
}

#endif
