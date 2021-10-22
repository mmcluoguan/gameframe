#ifndef DBVISIT_STDINHANDLER_H
#define DBVISIT_STDINHANDLER_H

#include "shynet/events/eventhandler.h"

namespace dbvisit {
/// <summary>
/// 终端输入处理
/// </summary>
class StdinHandler : public events::EventHandler {
public:
    StdinHandler(std::shared_ptr<events::EventBase> base, evutil_socket_t fd);
    ~StdinHandler();
    void input(int fd) override;

private:
    void quit_order(const char* od, int argc, char** argv, const char* optarg);
    void info_order(const char* od, int argc, char** argv, const char* optarg);
};
}

#endif
