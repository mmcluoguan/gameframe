#include "shynet/net/connectheartbeat.h"
#include "shynet/net/connectevent.h"
#include "shynet/net/timerreactormgr.h"
#include "shynet/utils/elapsed.h"

namespace shynet {
namespace net {
    ConnectHeartbeat::ConnectHeartbeat(std::weak_ptr<ConnectEvent> cnv, const struct timeval val)
        : TimerEvent(val, EV_TIMEOUT)
    {
        cnv_ = cnv;
    }

    void ConnectHeartbeat::timeout()
    {
        auto cb = [&]() {
            utils::Singleton<TimerReactorMgr>::instance().remove(timerid());
            std::shared_ptr<ConnectEvent> cnv = cnv_.lock();
            if (cnv != nullptr) {
                cnv->timerout(net::CloseType::TIMEOUT_CLOSE);
            }
        };
#ifdef USE_DEBUG
        shynet::utils::elapsed("工作线程计时单任务执行 ConnectHeartbeat");
        return cb();
#else
        return cb();
#endif
    }

}
}
