#include "shynet/signal/signalhandler.h"
#include "shynet/utils/logger.h"
#include <csignal>
#include <readline/readline.h>

namespace shynet {
namespace signal {
    void SignalHandler::add(std::shared_ptr<events::EventBase> base,
        int sig, callback cb)
    {
        std::shared_ptr<SignalEvent> se = std::make_shared<SignalEvent>(base, sig, cb);
        base->addevent(se, nullptr);
        sigevents_[sig] = se;
    }

    SignalHandler::SignalEvent::SignalEvent(std::shared_ptr<events::EventBase> base,
        int sig, callback cb)
        : events::EventHandler(base, sig, EV_SIGNAL | EV_PERSIST)
    {
        cb_ = cb;
    }

    void SignalHandler::SignalEvent::signal(int signal)
    {
        if (signal == SIGINT || signal == SIGQUIT) {
            rl_callback_handler_remove();
        }
        cb_(base(), signal);
    }
}
}
