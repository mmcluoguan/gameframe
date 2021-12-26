#ifndef ADMIN_CLIENT_CONSOLEHANDLER_H
#define ADMIN_CLIENT_CONSOLEHANDLER_H

#include "shynet/io/stdinhandler.h"

namespace admin_client {
/**
 * @brief 终端输入
*/
class ConsoleHandler : public shynet::io::StdinHandler {

    friend class shynet::utils::Singleton<ConsoleHandler>;
    /**
     * @brief 构造
     * @param base 反应器 
    */
    ConsoleHandler(std::shared_ptr<events::EventBase> base);

public:
    ~ConsoleHandler() = default;

private:
    /**
    * @brief 获取区服信息
    * @param order 命令字符串
    * @param argc 命令参数
    * @param argv 描述字符串
    */
    void getgamelist(const OrderItem& order, int argc, char** argv);
    /**
     * @brief 广播通知消息
     * @param order 命令字符串
     * @param argc 命令参数
     * @param argv 描述字符串
    */
    void noticeserver(const OrderItem& order, int argc, char** argv);
    /**
     * @brief 系统邮件
     * @param order 命令字符串
     * @param argc 命令参数
     * @param argv 描述字符串
    */
    void sysemail(const OrderItem& order, int argc, char** argv);
};
}

#endif
