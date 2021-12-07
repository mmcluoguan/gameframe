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
    class SignalHandler {

        friend class utils::Singleton<SignalHandler>;

    public:
        using callback = std::function<void(std::shared_ptr<events::EventBase>, int)>;

        ~SignalHandler() = default;
        void add(std::shared_ptr<events::EventBase> base,
            int sig,
            callback cb);

    private:
        class SignalEvent : public events::EventHandler {

        public:
            SignalEvent(std::shared_ptr<events::EventBase> base,
                int sig, callback cb);
            ~SignalEvent() = default;

        private:
            void signal(int signal) override;
            callback cb_;
        };

        explicit SignalHandler() = default;

    private:
        std::unordered_map<int, std::shared_ptr<SignalEvent>> sigevents_;
    };
}
}
#endif
