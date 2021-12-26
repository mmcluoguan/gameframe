#include "admin_client/consolehandler.h"
#include "admin_client//connectormgr.h"
#include "admin_client/worldconnector.h"
#include "shynet/net/connectreactormgr.h"
#include "shynet/utils/iniconfig.h"
#include "shynet/utils/singleton.h"
#include "shynet/utils/stringop.h"
#include "shynet/utils/stuff.h"
#include <chrono>
#include <cstring>
#include <thread>
#include <tuple>
#include <unistd.h>

extern int optind, opterr, optopt;
extern char* optarg;

namespace admin_client {

ConsoleHandler::ConsoleHandler(std::shared_ptr<events::EventBase> base)
    : shynet::io::StdinHandler(base)
{
    orderitems_.push_back({ "getgamelist", ":", "获取区服列表",
        std::bind(&ConsoleHandler::getgamelist, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3) });
    orderitems_.push_back({ "noticeserver", ":s:i:", "广播通知消息 s:区服sid,0表示全服 i:信息",
        std::bind(&ConsoleHandler::noticeserver, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3) });
    orderitems_.push_back({ "sysemail", ":j:", "发送系统邮件 j:json字符串",
        std::bind(&ConsoleHandler::sysemail, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3) });
}

void ConsoleHandler::getgamelist(const OrderItem& order, int argc, char** argv)
{
    std::shared_ptr<WorldConnector> world = shynet::utils::Singleton<ConnectorMgr>::instance().world_connector();
    if (world != nullptr) {
        world->send_json(frmpub::JosnMsgId::GETGAMELIST_ADMIN_WORLD_C);
        LOG_DEBUG << "发送" << frmpub::Basic::msgname(frmpub::JosnMsgId::GETGAMELIST_ADMIN_WORLD_C);
    } else {
        LOG_DEBUG << "没有可选的world连接";
    }
}

void ConsoleHandler::noticeserver(const OrderItem& order, int argc, char** argv)
{
    long sid = -1;
    char* info = nullptr;
    int opt;
    optind = 1;
    while ((opt = getopt(argc, argv, order.argstr)) != -1) {
        switch (opt) {
        case 's':
            char* p;
            sid = strtol(optarg, &p, 10);
            if (p == optarg) {
                sid = -1;
                LOG_WARN_BASE << "s的参数 " << optarg << " 不是数字";
            }
            break;
        case 'i':
            info = optarg;
            break;
        case ':':
            LOG_WARN_BASE << order.name << " (-" << (char)optopt << ") 丢失参数 ";
            break;
        case '?':
            LOG_WARN_BASE << order.name << " (-" << (char)optopt << ") 未知选项 ";
            break;
        default:
            THROW_EXCEPTION("call getopt");
        }
    }
    if (info == nullptr || sid == -1) {
        LOG_WARN_BASE << "正确格式参考:" << order.name << " " << order.argstr;
        return;
    }
    std::shared_ptr<WorldConnector> world = shynet::utils::Singleton<ConnectorMgr>::instance().world_connector();
    if (world != nullptr) {
        rapidjson::Document doc;
        rapidjson::Value msgc(rapidjson::kObjectType);
        msgc.AddMember("sid", sid, doc.GetAllocator());
        msgc.AddMember("info", rapidjson::StringRef(info), doc.GetAllocator());
        world->send_json(frmpub::JosnMsgId::NOTICESERVER_ADMIN_WORLD_C, &msgc);
        LOG_DEBUG << "发送" << frmpub::Basic::msgname(frmpub::JosnMsgId::NOTICESERVER_ADMIN_WORLD_C);
    } else {
        LOG_DEBUG << "没有可用的world连接";
    }
}

void ConsoleHandler::sysemail(const OrderItem& order, int argc, char** argv)
{
    char* josnstr = nullptr;
    int opt;
    optind = 1;
    while ((opt = getopt(argc, argv, order.argstr)) != -1) {
        switch (opt) {
        case 'j':
            josnstr = optarg;
            break;
        case ':':
            LOG_WARN_BASE << order.name << " (-" << (char)optopt << ") 丢失参数 ";
            break;
        case '?':
            LOG_WARN_BASE << order.name << " (-" << (char)optopt << ") 未知选项 ";
            break;
        default:
            THROW_EXCEPTION("call getopt");
        }
    }
    if (josnstr == nullptr) {
        LOG_WARN_BASE << "正确格式参考:" << order.name << " " << order.argstr;
        return;
    }
    std::shared_ptr<WorldConnector> world = shynet::utils::Singleton<ConnectorMgr>::instance().world_connector();
    if (world != nullptr) {
        rapidjson::Document doc;
        rapidjson::Value& msgc = doc.Parse(josnstr);
        if (msgc.IsObject()) {
            world->send_json(frmpub::JosnMsgId::SYSEMAIL_ADMIN_WORLD_C, &msgc);
            LOG_DEBUG << "发送" << frmpub::Basic::msgname(frmpub::JosnMsgId::SYSEMAIL_ADMIN_WORLD_C);
        } else {
            LOG_DEBUG << "输入参数值不是合法的json格式";
        }
    } else {
        LOG_DEBUG << "没有可用的world连接";
    }
}

}
