#include "frmpub/pingtimer.h"
#include "frmpub/connector.h"
#include "shynet/net/timerreactormgr.h"
#include "shynet/utils/elapsed.h"

namespace frmpub {
PingTimer::PingTimer(const timeval val, Connector* connector)
    : net::TimerEvent(val, EV_TIMEOUT | EV_PERSIST)
{
    connector_ = connector;
}
void PingTimer::timeout()
{
    auto cb = [&]() {
        if (connector_ != nullptr) {
            connector_->ping();
        } else {
            shynet::utils::Singleton<net::TimerReactorMgr>::instance().remove(timerid());
        }
    };
#ifdef USE_DEBUG
    shynet::utils::elapsed("工作线程计时单任务执行 PingTimer");
    return cb();
#else
    return cb();
#endif
}
}
