#ifndef CLIENT_FRMSTDINHANDLER_H
#define CLIENT_FRMSTDINHANDLER_H

#include "shynet/io/stdinhandler.h"

namespace frmpub {
/**
 * @brief ÖÕ¶ËÊäÈë
*/
class FrmStdinhandler : public shynet::io::StdinHandler {

    friend class shynet::utils::Singleton<FrmStdinhandler>;
    FrmStdinhandler(std::shared_ptr<events::EventBase> base);

public:
    /**
         * @brief ÀàĞÍÃû³Æ
        */
    static constexpr const char* kClassname = "FrmStdinhandler";

    ~FrmStdinhandler() = default;

private:
    /**
     * @brief ¿ªÆôluaÔ¶³Ìµ÷ÊÔ
     * @param order ÃüÁî×Ö·û´®
     * @param argc ÃüÁî²ÎÊı
     * @param argv ÃèÊö×Ö·û´®
    */
    void luadebugon(const OrderItem& order, int argc, char** argv);
    /**
     * @brief ¹Ø±ÕluaÔ¶³Ìµ÷ÊÔ
     * @param order ÃüÁî×Ö·û´®
     * @param argc ÃüÁî²ÎÊı
     * @param argv ÃèÊö×Ö·û´®
    */
    void luadebugoff(const OrderItem& order, int argc, char** argv);
};
}

#endif
