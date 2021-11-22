#ifndef CLIENT_GATERECONNCTORTIMER_H
#define CLIENT_GATERECONNCTORTIMER_H

#include "client/gateconnector.h"
#include "shynet/net/ipaddress.h"
#include "shynet/net/timerevent.h"

namespace client {
class GateReConnctorTimer : public net::TimerEvent {
public:
    GateReConnctorTimer(
        std::shared_ptr<net::IPAddress> connect_addr,
        std::shared_ptr<GateConnector::DisConnectData> data,
        const struct timeval val);
    ~GateReConnctorTimer();

    void timeout() override;

private:
    std::shared_ptr<net::IPAddress> connect_addr_;
    std::shared_ptr<GateConnector::DisConnectData> data_;
};
}

#endif
