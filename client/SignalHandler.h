#ifndef CLIENT_SIGINTHANDLER_H
#define CLIENT_SIGINTHANDLER_H

#include "shynet/events/EventHandler.h"

namespace client {
class SignalHandler : public events::EventHandler {
public:
    explicit SignalHandler(std::shared_ptr<events::EventBase> base);
    ~SignalHandler();
    void signal(int signal) override;
};
}

#endif
