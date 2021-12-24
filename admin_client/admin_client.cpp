#include "admin_client/consolehandler.h"
#include "admin_client/worldconnector.h"
#include "shynet/events/eventhandler.h"
#include "shynet/lua/luaengine.h"
#include "shynet/net/connectreactormgr.h"
#include "shynet/net/ipaddress.h"
#include "shynet/pool/threadpool.h"
#include "shynet/signal/signalhandler.h"
#include "shynet/utils/iniconfig.h"
#include "shynet/utils/stringop.h"
#include "shynet/utils/stuff.h"
#include <iomanip>
#include <unistd.h>

int main(int argc, char* argv[])
{
    using namespace std;
    using namespace shynet;
    using namespace shynet::utils;
    using namespace shynet::events;
    using namespace shynet::pool;
    using namespace shynet::net;
    using namespace shynet::lua;
    using namespace shynet::signal;
    using namespace frmpub;
    using namespace admin_client;
    try {
        const char* inifile = "gameframe.ini";
        IniConfig& ini = Singleton<IniConfig>::instance(inifile);
        stuff::create_coredump();
        Logger::set_loglevel(Logger::LogLevel::DEBUG);
        if (EventBase::usethread() == -1) {
            THROW_EXCEPTION("call usethread");
        }
        EventBase::initssl();

        Singleton<ThreadPool>::instance().start();

        //连接world_http服务器
        string worldstr = ini.get<string>("admin_client", "world");
        auto worldlist = stringop::split(worldstr, ",");
        if (worldlist.size() > 2 || worldlist.size() == 0) {
            THROW_EXCEPTION("world配置错误");
        }
        for (auto& item : worldlist) {
            std::string worldip = ini.get<string>(item, "http_ip");
            short worldport = ini.get<short>(item, "http_port");
            Singleton<ConnectReactorMgr>::instance().add(
                std::shared_ptr<WorldConnector>(
                    new WorldConnector(std::shared_ptr<IPAddress>(
                        new IPAddress(worldip.c_str(), worldport)))));
        }

        shared_ptr<EventBase> base(new EventBase());
        ConsoleHandler* stdin = &Singleton<ConsoleHandler>::instance(base);
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
