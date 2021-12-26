#ifndef SHYNET_LUATASK_TASK_H
#define SHYNET_LUATASK_TASK_H

#include "shynet/thread/thread.h"

namespace shynet {
namespace luatask {
    /**
     * @brief lua����
    */
    class LuaTask : public Nocopy {
    public:
        LuaTask() = default;
        ~LuaTask() = default;

        /**
         * @brief ����lua����
         * @param tif �߳���Ϣ
         * @return 0�ɹ�,-1ʧ��,����ʧ��lua�̻߳����
        */
        virtual int run(thread::Thread* tif) = 0;
    };
}
}

#endif
