#ifndef CLIENT_FRMSTDINHANDLER_H
#define CLIENT_FRMSTDINHANDLER_H

#include "shynet/io/stdinhandler.h"

namespace frmpub {
class FrmStdinhandler : public shynet::io::StdinHandler {

    friend class shynet::utils::Singleton<FrmStdinhandler>;
    FrmStdinhandler(std::shared_ptr<events::EventBase> base);

public:
    ~FrmStdinhandler();

private:
    void luadebugon(const OrderItem& order, int argc, char** argv);
    void luadebugoff(const OrderItem& order, int argc, char** argv);
};
}

#endif
