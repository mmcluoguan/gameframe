#include "shynet/task/connectreadiotask.h"

namespace shynet {
namespace task {

    ConnectReadIoTask::ConnectReadIoTask(std::shared_ptr<net::ConnectEvent> cntevent,
        std::unique_ptr<char[]> complete_data, size_t length)
    {
        cntevent_ = cntevent;
        complete_data_ = std::move(complete_data);
        complete_data_length_ = length;
    }

    void ConnectReadIoTask::operator()()
    {
        int ret = cntevent_->message_handle(complete_data_.get(), complete_data_length_);
        if (ret == -1) {
            cntevent_->close(net::CloseType::CLIENT_CLOSE);
        }
    }
}
}
