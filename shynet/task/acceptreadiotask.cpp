#include "shynet/task/acceptreadiotask.h"

namespace shynet {
namespace task {
    AcceptReadIoTask::AcceptReadIoTask(std::shared_ptr<net::AcceptNewFd> newfd,
        std::unique_ptr<char[]> complete_data, size_t length)
    {
        newfd_ = newfd;
        complete_data_ = std::move(complete_data);
        complete_data_length_ = length;
    }

    void AcceptReadIoTask::operator()()
    {
        int ret = newfd_->message_handle(complete_data_.get(), complete_data_length_);
        if (ret == -1) {
            newfd_->close(net::CloseType::SERVER_CLOSE);
        }
    }
}
}
