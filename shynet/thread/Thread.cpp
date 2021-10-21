#include "shynet/thread/Thread.h"

namespace shynet {
namespace thread {
    Thread::Thread(ThreadType t, size_t index)
    {
        type_ = t;
        index_ = index;
    }
    Thread::~Thread()
    {
    }

    std::shared_ptr<std::thread> Thread::start()
    {
        thread_ = std::make_shared<std::thread>(&Thread::run, this);
        return thread_;
    }
}
}
