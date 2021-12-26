#ifndef CLIENT_CONSOLEHANDLER_H
#define CLIENT_CONSOLEHANDLER_H

#include "shynet/io/stdinhandler.h"

namespace client {
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
    * @brief 重连服务器
    * @param order 命令字符串
    * @param argc 命令参数
    * @param argv 描述字符串
    */
    void reconnect_order(const OrderItem& order, int argc, char** argv);
    /**
    * @brief 登录
    * @param order 命令字符串
    * @param argc 命令参数
    * @param argv 描述字符串
    */
    void login_order(const OrderItem& order, int argc, char** argv);
    /**
     * @brief 发送gm消息
     * @param order 命令字符串
     * @param argc 命令参数
     * @param argv 描述字符串
    */
    void gm_order(const OrderItem& order, int argc, char** argv);
    /**
     * @brief 显示角色基础信息
     * @param order 命令字符串
     * @param argc 命令参数
     * @param argv 描述字符串
    */
    void display_role_base_order(const OrderItem& order, int argc, char** argv);
    /**
     * @brief 显示角色物品信息
     * @param order 命令字符串
     * @param argc 命令参数
     * @param argv 描述字符串
    */
    void display_role_goods_order(const OrderItem& order, int argc, char** argv);
    /**
     * @brief 发送设置角色等级
     * @param order 命令字符串
     * @param argc 命令参数
     * @param argv 描述字符串
    */
    void setlevel_order(const OrderItem& order, int argc, char** argv);
    /**
     * @brief 显示角色邮件列表
     * @param order 命令字符串
     * @param argc 命令参数
     * @param argv 描述字符串
    */
    void lookemail_order(const OrderItem& order, int argc, char** argv);
    /**
     * @brief 获取邮件附件
     * @param order 命令字符串
     * @param argc 命令参数
     * @param argv 描述字符串
    */
    void getannex_order(const OrderItem& order, int argc, char** argv);
};
}

#endif
