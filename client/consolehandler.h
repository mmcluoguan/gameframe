#ifndef CLIENT_CONSOLEHANDLER_H
#define CLIENT_CONSOLEHANDLER_H

#include "shynet/io/stdinhandler.h"

namespace client {
class ConsoleHandler : public shynet::io::StdinHandler {

    friend class shynet::utils::Singleton<ConsoleHandler>;
    ConsoleHandler(std::shared_ptr<events::EventBase> base);

public:
    ~ConsoleHandler();

private:
    void reconnect_order(const OrderItem& order, int argc, char** argv);
    void login_order(const OrderItem& order, int argc, char** argv);
    void gm_order(const OrderItem& order, int argc, char** argv);
};
}

#endif
