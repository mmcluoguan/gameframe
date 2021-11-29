#include "frmpub/frmstdinhandler.h"
#include "login/connectormgr.h"
#include "login/dbconnector.h"
#include "login/loginserver.h"
#include "login/luawrapper.h"
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
#include "sys/stat.h"
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
    using namespace shynet::io;
    using namespace shynet::signal;
    using namespace frmpub;
    using namespace login;
    try {
        const char* inifile = "gameframe.ini";
        IniConfig& ini = Singleton<IniConfig>::instance(std::move(inifile));
        bool daemon = ini.get<bool>("login", "daemon");
        if (daemon) {
            stuff::daemon();
        }

        stuff::create_coredump();
        Logger::loglevel(Logger::LogLevel::DEBUG);
        if (EventBase::usethread() == -1) {
            THROW_EXCEPTION("call usethread");
        }
        EventBase::initssl();
        int sid = ini.get<int>("login", "sid");
        const char* pid_dir = "./pid/";
        if (access(pid_dir, F_OK) == -1) {
            mkdir(pid_dir, S_IRWXU);
        }
        std::string pidfile = stringop::str_format("./%s/login_%d.pid", pid_dir, sid);
        stuff::writepid(pidfile);

        Singleton<LuaEngine>::instance(std::make_shared<login::LuaWrapper>());
        Singleton<ThreadPool>::instance().start();

        LOG_DEBUG << "开启登录服服务器监听";
        std::string loginip = ini.get<std::string>("login", "ip");
        short loginport = ini.get<short>("login", "port");
        std::shared_ptr<IPAddress> loginaddr(new IPAddress(loginip.c_str(), loginport));
        std::shared_ptr<LoginServer> loginserver(new LoginServer(loginaddr));
        Singleton<ListenReactorMgr>::instance().add(loginserver);

        //连接db服务器
        string dbstr = ini.get<string>("login", "db");
        auto dblist = stringop::split(dbstr, ",");
        if (dblist.size() > 2 || dblist.size() == 0) {
            THROW_EXCEPTION("db配置错误");
        }
        for (auto& item : dblist) {
            string dbip = ini.get<string>(item, "ip");
            short dbport = ini.get<short>(item, "port");
            shared_ptr<IPAddress> dbaddr(new IPAddress(dbip.c_str(), dbport));
            Singleton<ConnectReactorMgr>::instance().add(
                shared_ptr<DbConnector>(
                    new DbConnector(shared_ptr<IPAddress>(
                        new IPAddress(dbip.c_str(), dbport)))));
        }

        //连接world服务器
        string worldstr = ini.get<string>("login", "world");
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

        shared_ptr<EventBase> base(new EventBase());
        FrmStdinhandler* stdin = &Singleton<FrmStdinhandler>::instance(base);
        SignalHandler* sigint = &Singleton<SignalHandler>::instance(base);
        base->addevent(stdin, nullptr);
        base->addevent(sigint, nullptr);
        base->dispatch();
    } catch (const std::exception& err) {
        utils::stuff::print_exception(err);
    }
    EventBase::cleanssl();
    EventBase::event_shutdown();
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
