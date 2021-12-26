#ifndef SHYNET_IO_STDINHANDLER_H
#define SHYNET_IO_STDINHANDLER_H

#include "shynet/events/eventhandler.h"
#include "shynet/utils/singleton.h"
#include <functional>
#include <vector>

namespace shynet {
namespace io {
    /**
     * @brief 终端输入
    */
    class StdinHandler : public events::EventHandler {

        friend class utils::Singleton<StdinHandler>;

    protected:
        /**
         * @brief 构造
         * @param base 反应器 
        */
        explicit StdinHandler(std::shared_ptr<events::EventBase> base);

    public:
        /**
         * @brief 类型名称
        */
        static constexpr const char* kClassname = "StdinHandler";

        /**
         * @brief 命令数据
        */
        struct OrderItem {
            /**
             * @brief 命令字符串
            */
            const char* name;
            /**
             * @brief 命令参数
            */
            const char* argstr;
            /**
             * @brief 描述字符串
            */
            const char* desc;
            /**
             * @brief 命令回调函数
            */
            std::function<void(const OrderItem& order, int argc, char** argv)> callback;
        };

        ~StdinHandler() = default;
        /**
         * @brief EV_READ回调，可以读数据回调
         * @param fd 文件描述符
        */
        void input(int fd) override;

    protected:
        /**
         * @brief 命令向量
        */
        std::vector<OrderItem> orderitems_;

    private:
        /**
         * @brief 应用退出命令处理
         * @param order 命令字符串
         * @param argc 命令参数
         * @param argv 描述字符串
        */
        void quit_order(const OrderItem& order, int argc, char** argv);
        /**
         * @brief 应用信息命令处理
         * @param order 命令字符串
         * @param argc 命令参数
         * @param argv 描述字符串
        */
        void info_order(const OrderItem& order, int argc, char** argv);
    };
}
}

#endif
