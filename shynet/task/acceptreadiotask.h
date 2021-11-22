#ifndef SHYNET_TASK_ACCEPTREADIOTASK_H
#define SHYNET_TASK_ACCEPTREADIOTASK_H

#include "shynet/net/acceptnewfd.h"
#include "shynet/task/task.h"
#include "shynet/thread/thread.h"

namespace shynet {
namespace task {
    class AcceptReadIoTask : public task::Task {
    public:
        explicit AcceptReadIoTask(std::weak_ptr<net::AcceptNewFd> newfd);
        ~AcceptReadIoTask();

        int run(thread::Thread* tif) override;

    private:
        std::weak_ptr<net::AcceptNewFd> newfd_;
    };
}
}

#endif
