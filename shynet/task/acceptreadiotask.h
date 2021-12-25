#ifndef SHYNET_TASK_ACCEPTREADIOTASK_H
#define SHYNET_TASK_ACCEPTREADIOTASK_H

#include "shynet/net/acceptnewfd.h"
#include "shynet/thread/thread.h"

namespace shynet {
namespace task {
    class AcceptReadIoTask {
    public:
        explicit AcceptReadIoTask(std::weak_ptr<net::AcceptNewFd> newfd);
        ~AcceptReadIoTask();

        void operator()();

    private:
        std::weak_ptr<net::AcceptNewFd> newfd_;
    };
}
}

#endif
