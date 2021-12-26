#ifndef SHYNET_IO_STDINHANDLER_H
#define SHYNET_IO_STDINHANDLER_H

#include "shynet/events/eventhandler.h"
#include "shynet/utils/singleton.h"
#include <functional>
#include <vector>

namespace shynet {
namespace io {
    /**
     * @brief �ն�����
    */
    class StdinHandler : public events::EventHandler {

        friend class utils::Singleton<StdinHandler>;

    protected:
        /**
         * @brief ����
         * @param base ��Ӧ�� 
        */
        explicit StdinHandler(std::shared_ptr<events::EventBase> base);

    public:
        /**
         * @brief ��������
        */
        static constexpr const char* kClassname = "StdinHandler";

        /**
         * @brief ��������
        */
        struct OrderItem {
            /**
             * @brief �����ַ���
            */
            const char* name;
            /**
             * @brief �������
            */
            const char* argstr;
            /**
             * @brief �����ַ���
            */
            const char* desc;
            /**
             * @brief ����ص�����
            */
            std::function<void(const OrderItem& order, int argc, char** argv)> callback;
        };

        ~StdinHandler() = default;
        /**
         * @brief EV_READ�ص������Զ����ݻص�
         * @param fd �ļ�������
        */
        void input(int fd) override;

    protected:
        /**
         * @brief ��������
        */
        std::vector<OrderItem> orderitems_;

    private:
        /**
         * @brief Ӧ���˳������
         * @param order �����ַ���
         * @param argc �������
         * @param argv �����ַ���
        */
        void quit_order(const OrderItem& order, int argc, char** argv);
        /**
         * @brief Ӧ����Ϣ�����
         * @param order �����ַ���
         * @param argc �������
         * @param argv �����ַ���
        */
        void info_order(const OrderItem& order, int argc, char** argv);
    };
}
}

#endif
