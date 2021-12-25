#ifndef SHYNET_TASK_CONNECTREADIOTASK_H
#define SHYNET_TASK_CONNECTREADIOTASK_H

#include "shynet/net/connectevent.h"
#include "shynet/thread/thread.h"

namespace shynet {
namespace task {
    class ConnectReadIoTask {
    public:
        explicit ConnectReadIoTask(std::shared_ptr<net::ConnectEvent> cntevent);
        ~ConnectReadIoTask();

        void operator()();

    private:
        std::shared_ptr<net::ConnectEvent> cntevent_;
    };
}
}

#endif
