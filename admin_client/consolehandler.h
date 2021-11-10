#ifndef ADMIN_CLIENT_CONSOLEHANDLER_H
#define ADMIN_CLIENT_CONSOLEHANDLER_H

#include "shynet/io/stdinhandler.h"

namespace admin_client {
class ConsoleHandler : public shynet::io::StdinHandler {

    friend class shynet::utils::Singleton<ConsoleHandler>;
    ConsoleHandler(std::shared_ptr<events::EventBase> base);

public:
    ~ConsoleHandler();

private:
    void getgamelist(const OrderItem& order, int argc, char** argv);
    void noticeserver(const OrderItem& order, int argc, char** argv);
};
}

#endif
