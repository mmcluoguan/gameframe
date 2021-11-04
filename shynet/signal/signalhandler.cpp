#include "shynet/signal/signalhandler.h"
#include "shynet/utils/logger.h"
#include <csignal>
#include <readline/readline.h>

namespace shynet {
namespace signal {
    SignalHandler::SignalHandler(std::shared_ptr<events::EventBase> base)
        : events::EventHandler(base, SIGINT, EV_SIGNAL | EV_PERSIST)
    {
    }

    SignalHandler::~SignalHandler()
    {
    }

    void SignalHandler::signal(int signal)
    {
        if (signalmap_.size() == 0) {
            rl_callback_handler_remove();
            struct timeval delay = { 2, 0 };
            LOG_INFO << "捕获到中断信号,程序将在2秒后安全退出";
            base()->loopexit(&delay);
        } else {
            for (auto&& [signalnum, cb] : signalmap_) {
                if (signalnum == signal) {
                    cb(signal);
                }
            }
        }
    }
}
}
