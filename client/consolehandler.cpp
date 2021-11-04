#include "client/consolehandler.h"
#include "client/gateconnector.h"
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
extern int g_gateconnect_id;

namespace client {

ConsoleHandler::ConsoleHandler(std::shared_ptr<events::EventBase> base)
    : shynet::io::StdinHandler(base)
{
    orderitems_.push_back({ "reconnect", ":", "重连",
        std::bind(&ConsoleHandler::reconnect_order, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3) });

    orderitems_.push_back({ "login", ":p:", "登录 p:平台标识",
        std::bind(&ConsoleHandler::login_order, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3) });

    orderitems_.push_back({ "gm", ":o:a:", "GM指令 o:指令 a:参数组",
        std::bind(&ConsoleHandler::gm_order, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3) });
}

ConsoleHandler::~ConsoleHandler()
{
}

void ConsoleHandler::reconnect_order(const OrderItem& order, int argc, char** argv)
{
    std::shared_ptr<GateConnector> gate = std::dynamic_pointer_cast<GateConnector>(
        shynet::utils::Singleton<net::ConnectReactorMgr>::instance().find(g_gateconnect_id));
    if (gate != nullptr) {
        std::shared_ptr<GateConnector::DisConnectData> ptr = gate->disconnect_data();
        gate->close(net::ConnectEvent::CloseType::CLIENT_CLOSE);
        gate.reset();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        shynet::utils::IniConfig& ini = shynet::utils::Singleton<shynet::utils::IniConfig>::get_instance();
        std::string gateip = ini.get<const char*, std::string>("gate", "ip", "127.0.0.1");
        short gateport = ini.get<short, short>("gate", "port", short(25000));
        std::shared_ptr<net::IPAddress> gateaddr(new net::IPAddress(gateip.c_str(), gateport));
        std::shared_ptr<GateConnector> gateconnect(new GateConnector(gateaddr, ptr));
        gateaddr.reset();
        g_gateconnect_id = shynet::utils::Singleton<net::ConnectReactorMgr>::instance().add(gateconnect);
        gateconnect.reset();
    } else {
        LOG_DEBUG << "连接已经释放";
    }
}

void ConsoleHandler::login_order(const OrderItem& order, int argc, char** argv)
{
    int opt;
    optind = 1;
    char* platform = nullptr;
    while ((opt = getopt(argc, argv, order.argstr)) != -1) {
        switch (opt) {
        case 'p':
            platform = optarg;
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
    if (platform == nullptr) {
        LOG_WARN_BASE << "正确格式参考:" << order.name << " " << order.argstr;
        return;
    }
    protocc::login_client_gate_c msg;
    msg.set_platform_key(platform);

    std::shared_ptr<GateConnector> gate = std::dynamic_pointer_cast<GateConnector>(
        shynet::utils::Singleton<net::ConnectReactorMgr>::instance().find(g_gateconnect_id));
    if (gate != nullptr) {
        gate->send_proto(protocc::LOGIN_CLIENT_GATE_C, &msg);
        LOG_DEBUG << "发送" << frmpub::Basic::msgname(protocc::LOGIN_CLIENT_GATE_C);
    } else {
        LOG_WARN << "连接已经释放";
    }
}

void ConsoleHandler::gm_order(const OrderItem& order, int argc, char** argv)
{
    int opt;
    optind = 1;
    char* gm_name = nullptr;
    char* gm_args = nullptr;
    while ((opt = getopt(argc, argv, order.argstr)) != -1) {
        switch (opt) {
        case 'o':
            gm_name = optarg;
            break;
        case 'a':
            gm_args = optarg;
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
    if (gm_name == nullptr || gm_args == nullptr) {
        LOG_WARN_BASE << "正确格式参考:" << order.name << " " << order.argstr;
        return;
    }

    protocc::gmorder_client_gate_c msg;
    msg.set_order(gm_name);
    char* orderargv[20] = { 0 };
    int tmp = shynet::utils::StringOp::split(gm_args, ",", orderargv, 20);
    for (int i = 0; i < tmp; i++) {
        msg.add_args(orderargv[i]);
    }

    std::shared_ptr<GateConnector>
        gate = std::dynamic_pointer_cast<GateConnector>(
            shynet::utils::Singleton<net::ConnectReactorMgr>::instance().find(g_gateconnect_id));
    if (gate != nullptr) {
        msg.set_roleid(gate->roleid());
        gate->send_proto(protocc::GMORDER_CLIENT_GATE_C, &msg);
        LOG_DEBUG << "发送" << frmpub::Basic::msgname(protocc::GMORDER_CLIENT_GATE_C);
    } else {
        LOG_WARN << "连接已经释放";
    }
}

}
