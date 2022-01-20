#include "frmpub/frmstdinhandler.h"
#include "log/logclientmgr.h"
#include "log/logserver.h"
#include "log/logtimer.h"
#include "shynet/events/eventhandler.h"
#include "shynet/io/stdinhandler.h"
#include "shynet/net/ipaddress.h"
#include "shynet/net/timerreactormgr.h"
#include "shynet/signal/signalhandler.h"
#include "shynet/utils/filepathop.h"
#include "shynet/utils/idworker.h"
#include "shynet/utils/iniconfig.h"
#include "shynet/utils/stringop.h"
#include "shynet/utils/stuff.h"

//配置参数
const char* g_conf_node;
/**
 * @brief 写日志计时器
*/
std::weak_ptr<logs::LogTimer> g_logtimer;

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
    using namespace frmpub::protocc;
    using namespace logs;

    try {
        if (argc < 2) {
            THROW_EXCEPTION("没有配置参数");
        }
        g_conf_node = argv[1]; //节点名
        const char* inifile = "gameframe.ini";
        if (argc == 3) {
            inifile = argv[2]; //配置文件名
        }
        IniConfig& ini = Singleton<IniConfig>::instance(inifile);
        bool daemon = ini.get<bool>(g_conf_node, "daemon");
        if (argc == 4) {
            daemon = static_cast<bool>(std::stoi(argv[3])); //是否后台启动
        }
        if (daemon) {
            stuff::daemon();
        }
        stuff::create_coredump();
        Logger::set_loglevel(Logger::LogLevel::DEBUG);
        Logger::set_logname(g_conf_node);
        int centerid = ini.get<int>(g_conf_node, "centerid");
        int workerid = ini.get<int>(g_conf_node, "workerid");
        Singleton<IdWorker>::instance(workerid, centerid);

        string ip = ini.get<string>(g_conf_node, "ip");
        short port = ini.get<short>(g_conf_node, "port");
        //int sid = ini.get<int>(g_conf_node, "sid");
        string type = Basic::connectname(ServerType::LOG);
        string name = ini.get<string>(g_conf_node, "name");

        if (EventBase::usethread() == -1) {
            THROW_EXCEPTION("call usethread");
        }
        EventBase::initssl();

        Singleton<ThreadPool>::instance().start();

        shared_ptr<IPAddress> ipaddr(new IPAddress(ip.c_str(), port));
        shared_ptr<LogServer> logserver(new LogServer(ipaddr));
        Singleton<ListenReactorMgr>::instance().add(logserver);

        const char* pid_dir = "./pid/";
        filepathop::mkdir_recursive(pid_dir);
        std::string pidfile = stringop::str_format("./%s/%s.pid", pid_dir, g_conf_node);
        stuff::writepid(pidfile);

        auto logtimer = make_shared<LogTimer>(timeval { 5L, 0L });
        Singleton<TimerReactorMgr>::instance().add(logtimer);
        g_logtimer = logtimer;

        shared_ptr<EventBase> base(new EventBase());
        FrmStdinhandler* stdin = &Singleton<FrmStdinhandler>::instance(base);
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
