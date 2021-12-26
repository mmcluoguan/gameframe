#ifndef CLIENT_FRMSTDINHANDLER_H
#define CLIENT_FRMSTDINHANDLER_H

#include "shynet/io/stdinhandler.h"

namespace frmpub {
/**
 * @brief �ն�����
*/
class FrmStdinhandler : public shynet::io::StdinHandler {

    friend class shynet::utils::Singleton<FrmStdinhandler>;
    FrmStdinhandler(std::shared_ptr<events::EventBase> base);

public:
    /**
         * @brief ��������
        */
    static constexpr const char* kClassname = "FrmStdinhandler";

    ~FrmStdinhandler() = default;

private:
    /**
     * @brief ����luaԶ�̵���
     * @param order �����ַ���
     * @param argc �������
     * @param argv �����ַ���
    */
    void luadebugon(const OrderItem& order, int argc, char** argv);
    /**
     * @brief �ر�luaԶ�̵���
     * @param order �����ַ���
     * @param argc �������
     * @param argv �����ַ���
    */
    void luadebugoff(const OrderItem& order, int argc, char** argv);
};
}

#endif
