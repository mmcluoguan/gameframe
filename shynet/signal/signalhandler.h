#ifndef SHYNET_SIGNAL_SIGINTHANDLER_H
#define SHYNET_SIGNAL_SIGINTHANDLER_H

#include "shynet/events/eventhandler.h"
#include "shynet/utils/singleton.h"
#include <functional>
#include <unordered_map>

namespace shynet {
namespace signal {

    /// <summary>
    /// 系统信号处理
    /// </summary>
    class SignalHandler : public events::EventHandler {

        friend class utils::Singleton<SignalHandler>;

    public:
        ~SignalHandler();

    private:
        explicit SignalHandler(std::shared_ptr<events::EventBase> base);
        void signal(int signal) override;

    protected:
        std::unordered_map<int, std::function<void(int)>> signalmap_;
    };
}
}
#endif
