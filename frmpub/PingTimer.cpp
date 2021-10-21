#include "frmpub/PingTimer.h"
#include "frmpub/Connector.h"
#include "shynet/net/TimerReactorMgr.h"

namespace frmpub {
PingTimer::PingTimer(const timeval val, Connector* connector)
    : net::TimerEvent(val, EV_TIMEOUT | EV_PERSIST)
{
    connector_ = connector;
}
PingTimer::~PingTimer()
{
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
