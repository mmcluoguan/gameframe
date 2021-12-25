#include "shynet/task/acceptreadiotask.h"

namespace shynet {
namespace task {
    AcceptReadIoTask::AcceptReadIoTask(std::weak_ptr<net::AcceptNewFd> newfd)
    {
        newfd_ = newfd;
    }
    AcceptReadIoTask::~AcceptReadIoTask()
    {
    }

    void AcceptReadIoTask::operator()()
    {
        std::shared_ptr<net::AcceptNewFd> aptnewfd = newfd_.lock();
        if (aptnewfd != nullptr) {
            std::shared_ptr<net::AcceptHeartbeat> ht = aptnewfd->heart().lock();
            if (ht != nullptr) {
                ht->set_val({ aptnewfd->heart_second(), 0 });
            }
            int ret = aptnewfd->input();
            if (ret == -1) {
                aptnewfd->close(true);
            } else if (ret == -2) {
                aptnewfd->close(false);
            }
        }
    }
}
}
