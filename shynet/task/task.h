#ifndef SHYNET_TASK_TASK_H
#define SHYNET_TASK_TASK_H

#include "shynet/thread/thread.h"

namespace shynet {
namespace task {
    class Task : public Nocopy {
    public:
        Task();
        ~Task();

        virtual int run(thread::Thread* tif) = 0;
    };
}
}

#endif
