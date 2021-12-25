#include "shynet/task/connectreadiotask.h"

namespace shynet {
namespace task {

    ConnectReadIoTask::ConnectReadIoTask(std::shared_ptr<net::ConnectEvent> cntevent)
    {
        cntevent_ = cntevent;
    }
    ConnectReadIoTask::~ConnectReadIoTask()
    {
    }

    void ConnectReadIoTask::operator()()
    {
        std::shared_ptr<net::ConnectHeartbeat> ht = cntevent_->heart().lock();
        if (ht != nullptr) {
            ht->set_val({ cntevent_->heart_second(), 0 });
        }
        int ret = cntevent_->input();
        if (ret == -1) {
            cntevent_->close(net::ConnectEvent::CloseType::CLIENT_CLOSE);
        } else if (ret == -2) {
            cntevent_->close(net::ConnectEvent::CloseType::SERVER_CLOSE);
        }
    }
}
}
