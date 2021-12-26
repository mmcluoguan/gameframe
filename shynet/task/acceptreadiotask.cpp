#include "shynet/task/acceptreadiotask.h"

namespace shynet {
namespace task {
    AcceptReadIoTask::AcceptReadIoTask(std::weak_ptr<net::AcceptNewFd> newfd)
    {
        newfd_ = newfd;
    }

    void AcceptReadIoTask::operator()()
    {
        std::shared_ptr<net::AcceptNewFd> aptnewfd = newfd_.lock();
        if (aptnewfd != nullptr) {
            std::shared_ptr<net::AcceptHeartbeat> ht = aptnewfd->heart().lock();
            if (ht != nullptr) {
                ht->set_val({ aptnewfd->heart_second(), 0 });
            }
            net::InputResult ret = aptnewfd->input();
            if (ret == net::InputResult::INITIATIVE_CLOSE) {
                aptnewfd->close(net::CloseType::SERVER_CLOSE);
            } else if (ret == net::InputResult::PASSIVE_CLOSE) {
                aptnewfd->close(net::CloseType::CLIENT_CLOSE);
            }
        }
    }
}
}
