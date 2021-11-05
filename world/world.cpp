#include "shynet/events/eventhandler.h"
#include "shynet/io/stdinhandler.h"
#include "shynet/lua/luaengine.h"
#include "shynet/net/connectreactormgr.h"
#include "shynet/net/ipaddress.h"
#include "shynet/pool/threadpool.h"
#include "shynet/signal/signalhandler.h"
#include "shynet/utils/iniconfig.h"
#include "shynet/utils/stringop.h"
#include "shynet/utils/stuff.h"
#include "world/dbconnector.h"
#include "world/httpserver.h"
#include "world/luawrapper.h"
#include "world/worldserver.h"
#include <sys/stat.h>
#include <unistd.h>

//配置参数
const char* g_confname;

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
    using namespace world;

    try {
        if (argc < 2) {
            THROW_EXCEPTION("没有配置参数");
        }
        g_confname = argv[1];

        const char* file = "gameframe.ini";
        IniConfig& ini = Singleton<IniConfig>::instance(std::move(file));
        bool daemon = ini.get<bool, bool>(g_confname, "daemon", false);
        if (daemon) {
            Stuff::daemon();
            Singleton<IniConfig>::instance(std::move(string("gameframe.ini").c_str()));
        }

        Stuff::create_coredump();
        Logger::loglevel(Logger::LogLevel::DEBUG);
        if (EventBase::usethread() == -1) {
            THROW_EXCEPTION("call usethread");
        }
        EventBase::initssl();
        const char* pid_dir = "./pid/";
        if (access(pid_dir, F_OK) == -1) {
            mkdir(pid_dir, S_IRWXU);
        }
        std::string pidfile = StringOp::str_format("./%s/%s.pid", pid_dir, g_confname);
        Stuff::writepid(pidfile);

        Singleton<LuaEngine>::instance(std::make_shared<world::LuaWrapper>());
        Singleton<ThreadPool>::instance().start();

        LOG_DEBUG << "开启世界服务器监听";
        std::string worldip = ini.get<const char*, std::string>(g_confname, "ip", "127.0.0.1");
        short worldport = ini.get<short, short>(g_confname, "port", short(22000));
        std::shared_ptr<IPAddress> worldaddr(new IPAddress(worldip.c_str(), worldport));
        std::shared_ptr<WorldServer> worldserver(new WorldServer(worldaddr));
        Singleton<ListenReactorMgr>::instance().add(worldserver);

        LOG_DEBUG << "开启http后台服务器监听";
        std::string httpip = ini.get<const char*, std::string>(g_confname, "http_ip", "127.0.0.1");
        short httpport = ini.get<short, short>(g_confname, "http_port", short(26000));
        std::shared_ptr<IPAddress> httpaddr(new IPAddress(httpip.c_str(), httpport));
        std::shared_ptr<HttpServer> httpserver(new HttpServer(httpaddr));
        Singleton<ListenReactorMgr>::instance().add(httpserver);

        //连接db服务器
        string dbstr = ini.get<const char*, string>(g_confname, "db", "");
        auto dblist = StringOp::split(dbstr, ",");
        if (dblist.size() > 2 || dblist.size() == 0) {
            THROW_EXCEPTION("db配置错误");
        }
        for (auto& item : dblist) {
            string dbip = ini.get<const char*, string>(item, "ip", "");
            short dbport = ini.get<short, short>(item, "port", short(21000));
            shared_ptr<IPAddress> dbaddr(new IPAddress(dbip.c_str(), dbport));
            Singleton<ConnectReactorMgr>::instance().add(
                shared_ptr<DbConnector>(
                    new DbConnector(shared_ptr<IPAddress>(
                        new IPAddress(dbip.c_str(), dbport)))));
        }

        shared_ptr<EventBase> base(new EventBase());
        StdinHandler* stdin = &Singleton<StdinHandler>::instance(base);
        SignalHandler* sigint = &Singleton<SignalHandler>::instance(base);
        base->addevent(stdin, nullptr);
        base->addevent(sigint, nullptr);
        base->dispatch();
    } catch (const std::exception& err) {
        utils::Stuff::print_exception(err);
    }
    EventBase::cleanssl();
    EventBase::event_shutdown();
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
