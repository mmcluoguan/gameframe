#ifndef LOGIN_SIGINTHANDLER_H
#define LOGIN_SIGINTHANDLER_H

#include "shynet/events/eventhandler.h"

namespace login {
/// <summary>
/// 系统信号处理
/// </summary>
class SignalHandler : public events::EventHandler {
public:
    explicit SignalHandler(std::shared_ptr<events::EventBase> base);
    ~SignalHandler();
    void signal(int signal) override;
};
}

#endif
