#ifndef SHYNET_LUATASK_TASK_H
#define SHYNET_LUATASK_TASK_H

#include "shynet/thread/thread.h"

namespace shynet {
namespace luatask {
    class LuaTask : public Nocopy {
    public:
        LuaTask();
        ~LuaTask();

        virtual int run(thread::Thread* tif) = 0;
    };
}
}

#endif
