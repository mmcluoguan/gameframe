#include "shynet/task/connectreadiotask.h"

namespace shynet {
namespace task {

    ConnectReadIoTask::ConnectReadIoTask(std::shared_ptr<net::ConnectEvent> cntevent)
    {
        cntevent_ = cntevent;
    }

    void ConnectReadIoTask::operator()()
    {
        net::InputResult ret = cntevent_->input();
        if (ret == net::InputResult::INITIATIVE_CLOSE) {
            cntevent_->close(net::CloseType::CLIENT_CLOSE);
        } else if (ret == net::InputResult::PASSIVE_CLOSE) {
            cntevent_->close(net::CloseType::SERVER_CLOSE);
        }
    }
}
}
