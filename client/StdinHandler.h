#ifndef CLIENT_STDINHANDLER_H
#define CLIENT_STDINHANDLER_H

#include "shynet/events/EventHandler.h"

namespace client {
class StdinHandler : public events::EventHandler {
public:
    StdinHandler(std::shared_ptr<events::EventBase> base, evutil_socket_t fd);
    ~StdinHandler();
    void input(int fd) override;

private:
    void quit_order(const char* od, int argc, char** argv, const char* optstr);
    void info_order(const char* od, int argc, char** argv, const char* optstr);
    void reconnect_order(const char* od, int argc, char** argv, const char* optstr);
    void login_order(const char* od, int argc, char** argv, const char* optstr);
};
}

#endif
