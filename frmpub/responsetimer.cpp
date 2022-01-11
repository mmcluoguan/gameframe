#include "frmpub/responsetimer.h"
#include "frmpub/filterdata.h"
#include "shynet/net/timerreactormgr.h"
#include "shynet/utils/elapsed.h"

namespace frmpub {
ResponseTimer::ResponseTimer(const timeval val,
    std::weak_ptr<FilterData> filter,
    int msgid_c, int msgid_s)
    : net::TimerEvent(val, EV_TIMEOUT)
{
    filter_ = filter;
    msgid_c_ = msgid_c;
    msgid_s_ = msgid_s;
}
void ResponseTimer::timeout()
{
    auto cb = [&]() {
        auto sh = filter_.lock();
        if (sh != nullptr) {
            sh->default_response_timeout(msgid_c_, msgid_s_);
        }
    };
#ifdef USE_DEBUG
    shynet::utils::elapsed("工作线程计时单任务执行 ResponseTimer");
    return cb();
#else
    return cb();
#endif
}
}
