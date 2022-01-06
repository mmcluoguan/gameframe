#include "shynet/net/acceptheartbeat.h"
#include "shynet/net/acceptnewfd.h"
#include "shynet/utils/elapsed.h"

namespace shynet {
namespace net {
    AcceptHeartbeat::AcceptHeartbeat(std::weak_ptr<AcceptNewFd> newfd, const struct timeval val)
        : TimerEvent(val, EV_TIMEOUT)
    {
        newfd_ = newfd;
    }

    void AcceptHeartbeat::timeout()
    {
        auto cb = [&]() {
            utils::Singleton<TimerReactorMgr>::instance().remove(timerid());
            std::shared_ptr<AcceptNewFd> newfd = newfd_.lock();
            if (newfd != nullptr) {
                newfd->timerout(net::CloseType::TIMEOUT_CLOSE);
            }
        };
#ifdef USE_DEBUG
        shynet::utils::elapsed("工作线程计时单任务执行 AcceptHeartbeat");
        return cb();
#else
        return cb();
#endif
    }
}
}
