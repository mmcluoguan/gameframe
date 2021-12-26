#include "frmpub/frmstdinhandler.h"
#include "frmpub/luaremotedebug.h"
#include <unistd.h>

extern int optind, opterr, optopt;
extern char* optarg;

namespace frmpub {

FrmStdinhandler::FrmStdinhandler(std::shared_ptr<events::EventBase> base)
    : shynet::io::StdinHandler(base)
{
    orderitems_.push_back({ "luadebugon", ":p:", "开启lua调试",
        std::bind(&FrmStdinhandler::luadebugon, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3) });
    orderitems_.push_back({ "luadebugoff", ":", "关闭lua调试",
        std::bind(&FrmStdinhandler::luadebugoff, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3) });
}

void FrmStdinhandler::luadebugon(const OrderItem& order, int argc, char** argv)
{
    int opt;
    optind = 1;
    char* debugip = nullptr;
    while ((opt = getopt(argc, argv, order.argstr)) != -1) {
        switch (opt) {
        case 'p':
            debugip = optarg;
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
    if (debugip == nullptr) {
        LOG_WARN_BASE << "正确格式参考:" << order.name << " " << order.argstr;
        return;
    }
    shynet::utils::Singleton<LuaRemoteDebug>::instance().enable(std::string(debugip));
    LOG_INFO_BASE << "luadebug 启动";
}

void FrmStdinhandler::luadebugoff(const OrderItem& order, int argc, char** argv)
{
    shynet::utils::Singleton<LuaRemoteDebug>::instance().disenable();
    LOG_INFO_BASE << "luadebug 关闭";
}

}
