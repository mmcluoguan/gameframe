#ifndef SHYNET_LUATASK_TASK_H
#define SHYNET_LUATASK_TASK_H

#include "shynet/thread/thread.h"

namespace shynet {
namespace luatask {
    /**
     * @brief lua任务
    */
    class LuaTask : public Nocopy {
    public:
        LuaTask() = default;
        ~LuaTask() = default;

        /**
         * @brief 运行lua任务
         * @param tif 线程信息
         * @return 0成功,-1失败,返回失败lua线程会结束
        */
        virtual int run(thread::Thread* tif) = 0;
    };
}
}

#endif
