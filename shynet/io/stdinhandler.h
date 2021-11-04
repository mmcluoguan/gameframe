#ifndef SHYNET_IO_STDINHANDLER_H
#define SHYNET_IO_STDINHANDLER_H

#include "shynet/events/eventhandler.h"
#include "shynet/utils/singleton.h"
#include <functional>
#include <vector>

namespace shynet {
namespace io {

    class StdinHandler : public events::EventHandler {

        friend class utils::Singleton<StdinHandler>;

    protected:
        StdinHandler(std::shared_ptr<events::EventBase> base);

    public:
        struct OrderItem {
            const char* name;
            const char* argstr;
            const char* desc;
            std::function<void(const OrderItem& order, int argc, char** argv)> callback;
        };

        ~StdinHandler();
        void input(int fd) override;

    protected:
        std::vector<OrderItem> orderitems_;

    private:
        void quit_order(const OrderItem& order, int argc, char** argv);
        void info_order(const OrderItem& order, int argc, char** argv);
    };
}
}

#endif
