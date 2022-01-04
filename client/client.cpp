#include "client/consolehandler.h"
#include "client/gateconnector.h"
#include "shynet/events/eventhandler.h"
#include "shynet/lua/luaengine.h"
#include "shynet/net/connectreactormgr.h"
#include "shynet/net/ipaddress.h"
#include "shynet/pool/threadpool.h"
#include "shynet/signal/signalhandler.h"
#include "shynet/utils/iniconfig.h"
#include "shynet/utils/stuff.h"
#include <unistd.h>

int g_gateconnect_id;

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
    using namespace client;
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

        for (int i = 0; i < 500; i++) {
            string gateip = ini.get<string>("client", "gateip");
            short gateport = ini.get<short>("client", "gateport");
            shared_ptr<IPAddress> gateaddr(new IPAddress(gateip.c_str(), gateport));
            shared_ptr<GateConnector> gateconnect(new GateConnector(gateaddr));
            gateconnect->set_platform_key("test_" + std::to_string(i));
            gateaddr.reset();
            g_gateconnect_id = Singleton<ConnectReactorMgr>::instance().add(gateconnect);
            gateconnect.reset();
            LOG_DEBUG << "计数:" << i;
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
