#include "frmpub/pingtimer.h"
#include "frmpub/connector.h"
#include "shynet/net/timerreactormgr.h"

namespace frmpub {
PingTimer::PingTimer(const timeval val, Connector* connector)
    : net::TimerEvent(val, EV_TIMEOUT | EV_PERSIST)
{
    connector_ = connector;
}
void PingTimer::timeout()
{
    if (connector_ != nullptr) {
        connector_->ping();
    } else {
        shynet::utils::Singleton<net::TimerReactorMgr>::instance().remove(timerid());
    }
}
}
