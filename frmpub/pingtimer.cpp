#include "frmpub/pingtimer.h"
#include "frmpub/connector.h"
#include "shynet/net/timerreactormgr.h"
#include "shynet/utils/elapsed.h"

namespace frmpub {
PingTimer::PingTimer(const timeval val, shynet::protocol::FilterProces* filter)
    : net::TimerEvent(val, EV_TIMEOUT | EV_PERSIST)
{
    filter_ = filter;
}
void PingTimer::timeout()
{
    auto cb = [&]() {
        if (filter_ != nullptr) {
            filter_->ping();
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
