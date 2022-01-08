#ifndef FRMPUB_BASIC_H
#define FRMPUB_BASIC_H

#include "3rd/curl/curl.h"
#include "3rd/enum/magic_enum.hpp"
#include "3rd/rapidjson/document.h"
#include "frmpub/protocc/client.pb.h"
#include "frmpub/protocc/common.pb.h"
#include "frmpub/protocc/dbvisit.pb.h"
#include "frmpub/protocc/game.pb.h"
#include "frmpub/protocc/gate.pb.h"
#include "frmpub/protocc/internal.pb.h"
#include "frmpub/protocc/log.pb.h"
#include "frmpub/protocc/login.pb.h"
#include "frmpub/protocc/world.pb.h"
#include "shynet/events/eventbuffer.h"
#include "shynet/utils/logger.h"
#include <csignal>
#include <stack>
#include <unordered_map>
namespace protocc = frmpub::protocc;

#ifndef SEND_ERR
#define SEND_ERR(code, str) \
    LOG_DEBUG << (str);     \
    send_errcode((code), (str));
#define SEND_ERR_EX(code, str, enves) \
    LOG_DEBUG << (str);               \
    send_errcode((code), (str), (enves));
#endif // !SEND_ERR

namespace frmpub {

enum JosnMsgId : int {
    ADMIN_WORLD_BEGIN = 5001,
    GETGAMELIST_ADMIN_WORLD_C, //获取区服信息
    GETGAMELIST_ADMIN_WORLD_S, //{"games":[{"ip":"127.0.0.1","port":1234,"st":1,"name":"xxxx"}]}
    NOTICESERVER_ADMIN_WORLD_C, //广播通知消息 {"sid":1,"info":"xxxxx"}
    NOTICESERVER_ADMIN_WORLD_S, //{"result":1}
    /*
    * title 标题
    * info 内容
    * type 类型 1后台邮件 2区服邮件
    * sid 区服id -1表示全服
    * rid 区服角色id -1表示全部角色
    * time 发送时间
    * annex 附件 gold游戏币 diamond钻石 goods物品列表
    */
    SYSEMAIL_ADMIN_WORLD_C, //发送系统邮件 {"title":"xxx","info":"xxx","type":1,"sid":-1,"rid":-1,"time":123213,"annex":{"gold":111,"diamond":111,"goods":[{"cfgid":10010,"num":10}]}}
    SYSEMAIL_ADMIN_WORLD_S, //{"result":1}

    ADMIN_WORLD_END = 5256,
};

rapidjson::Value& get_json_value(rapidjson::Value& jv, std::string key);
rapidjson::Value& get_json_value(rapidjson::Document& doc, std::string key);

class Basic {
public:
    /**
     * @brief 通过消息id获取消息名称
     * @param  msgid 消息id
     * @return 消息名称
    */
    static std::string msgname(int id);
    /**
     * @brief 通过服务类行获取服务名称
     * @param st 服务类行
     * @return 服务名称
    */
    static std::string connectname(protocc::ServerType st);

private:
    static std::string internal_msgname(int);
    static std::string client_msgname(int);
    static std::string json_msgname(int);
};

class Sms {
public:
    /**
     * @brief 发送短信
     * @param url 地址
     * @return 服务器响应字符串
    */
    static std::string send(std::string url);

private:
    static size_t req_reply(void* ptr, size_t size, size_t nmemb, void* stream);
};

/**
 * @brief 默认系统信号处理
 * @param base 反应器
 * @param signum 系统信号
*/
inline void default_sigcb(std::shared_ptr<events::EventBase> base, int signum)
{
    if (signum == SIGINT) {
        struct timeval delay = { 2, 0 };
        LOG_INFO << "捕获到中断信号,程序将在2秒后安全退出";
        base->loopexit(&delay);
    } else if (signum == SIGQUIT) {
        LOG_INFO << "捕获到退出信号,程序将在立刻安全退出";
        base->loopexit();
    }
}

/**
 * @brief 设置日志收集
*/
void set_loggather();
}

namespace magic_enum::customize {
template <>
struct enum_range<frmpub::JosnMsgId> {
    static constexpr int min = frmpub::ADMIN_WORLD_BEGIN;
    static constexpr int max = frmpub::ADMIN_WORLD_END;
};
}
#endif
