#include "gate/connectormgr.h"
#include "gate/gateclientmgr.h"
#include "gate/gateserver.h"
#include "gate/luawrapper.h"
#include "shynet/events/eventhandler.h"
#include "shynet/io/stdinhandler.h"
#include "shynet/lua/luaengine.h"
#include "shynet/net/connectreactormgr.h"
#include "shynet/net/ipaddress.h"
#include "shynet/pool/threadpool.h"
#include "shynet/signal/signalhandler.h"
#include "shynet/utils/filepathop.h"
#include "shynet/utils/iniconfig.h"
#include "shynet/utils/stringop.h"
#include "shynet/utils/stuff.h"
#include "sys/stat.h"
#include <unistd.h>

//配置参数
const char* g_conf_node;

int main(int argc, char* argv[])
{
    using namespace std;
    using namespace shynet;
    using namespace shynet::utils;
    using namespace shynet::events;
    using namespace shynet::pool;
    using namespace shynet::net;
    using namespace shynet::lua;
    using namespace shynet::io;
    using namespace shynet::signal;
    using namespace frmpub;
    using namespace gate;

    try {
        if (argc < 2) {
            THROW_EXCEPTION("没有配置参数");
        }
        g_conf_node = argv[1]; //节点名
        const char* inifile = "gameframe.ini";
        if (argc == 3) {
            inifile = argv[2]; //配置文件名
        }
        IniConfig& ini = Singleton<IniConfig>::instance(std::move(inifile));
        bool daemon = ini.get<bool>(g_conf_node, "daemon");
        if (daemon) {
            stuff::daemon();
        }
        stuff::create_coredump();
        Logger::set_loglevel(Logger::LogLevel::DEBUG);
        Logger::set_logname(g_conf_node);
        if (EventBase::usethread() == -1) {
            THROW_EXCEPTION("call usethread");
        }
        EventBase::initssl();
        //int sid = ini.get<int>(g_conf_node, "sid");

        Singleton<LuaEngine>::instance(std::make_shared<gate::LuaWrapper>());
        Singleton<ThreadPool>::instance().start();

        LOG_DEBUG << "开启网关服服务器监听";
        std::string gateip = ini.get<std::string>(g_conf_node, "ip");
        short gateport = ini.get<short>(g_conf_node, "port");
        std::shared_ptr<IPAddress> gateaddr(new IPAddress(gateip.c_str(), gateport));
        std::shared_ptr<GateServer> gateserver(new GateServer(gateaddr));
        Singleton<ListenReactorMgr>::instance().add(gateserver);

        //连接world服务器
        string worldstr = ini.get<string>(g_conf_node, "world");
        auto worldlist = stringop::split(worldstr, ",");
        if (worldlist.size() > 2 || worldlist.size() == 0) {
            THROW_EXCEPTION("world配置错误");
        }
        for (auto& item : worldlist) {
            std::string worldip = ini.get<string>(item, "ip");
            short worldport = ini.get<short>(item, "port");
            Singleton<ConnectReactorMgr>::instance().add(
                std::shared_ptr<WorldConnector>(
                    new WorldConnector(std::shared_ptr<IPAddress>(
                        new IPAddress(worldip.c_str(), worldport)))));
        }

        const char* pid_dir = "./pid/";
        filepathop::mkdir_recursive(pid_dir);
        std::string pidfile = stringop::str_format("./%s/%s.pid", pid_dir, g_conf_node);
        stuff::writepid(pidfile);
        shared_ptr<EventBase> base(new EventBase());

        StdinHandler* stdin = &Singleton<StdinHandler>::instance(base);
        base->addevent(stdin, nullptr);
        SignalHandler* sigmgr = &Singleton<SignalHandler>::instance();
        sigmgr->add(base, SIGINT, default_sigcb);
        sigmgr->add(base, SIGQUIT, default_sigcb);
        base->dispatch();
    } catch (const std::exception& err) {
        utils::stuff::print_exception(err);
    }
    EventBase::cleanssl();
    EventBase::event_shutdown();
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
