#include "shynet/net/connectheartbeat.h"
#include "shynet/net/connectevent.h"
#include "shynet/net/timerreactormgr.h"

namespace shynet {
namespace net {
    ConnectHeartbeat::ConnectHeartbeat(std::weak_ptr<ConnectEvent> cnv, const struct timeval val)
        : TimerEvent(val, EV_TIMEOUT)
    {
        cnv_ = cnv;
    }

    void ConnectHeartbeat::timeout()
    {
        utils::Singleton<TimerReactorMgr>::instance().remove(timerid());
        std::shared_ptr<ConnectEvent> cnv = cnv_.lock();
        if (cnv != nullptr) {
            cnv->timerout(net::CloseType::TIMEOUT_CLOSE);
        }
    }

}
}
