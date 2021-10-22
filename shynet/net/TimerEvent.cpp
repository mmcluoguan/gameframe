#include "shynet/net/timerevent.h"
#include "shynet/pool/threadpool.h"
#include "shynet/task/timertask.h"

namespace shynet {
namespace net {
    TimerEvent::TimerEvent(const struct timeval val, short what)
        : EventHandler()
        , val_(val)
    {
        what_ = what;
    }
    TimerEvent::~TimerEvent() { }

    void TimerEvent::set_val(const timeval& t)
    {
        if (event() != nullptr) {
            val_.tv_sec = t.tv_sec;
            val_.tv_usec = t.tv_usec;
            event_add(event(), &val_);
        }
    }

    void TimerEvent::timeout(int fd)
    {
        std::shared_ptr<task::TimerTask> ttk = std::make_shared<task::TimerTask>(timerid_);
        utils::Singleton<pool::ThreadPool>::instance().append(ttk);
    }

} // namespace net
} // namespace shynet
