#include "login/ConnectorMgr.h"
#include "login/DbConnector.h"
#include "login/LoginServer.h"
#include "login/LuaWrapper.h"
#include "login/SignalHandler.h"
#include "login/StdinHandler.h"
#include "shynet/events/EventHandler.h"
#include "shynet/lua/LuaEngine.h"
#include "shynet/net/ConnectReactorMgr.h"
#include "shynet/net/IPAddress.h"
#include "shynet/pool/ThreadPool.h"
#include "shynet/utils/IniConfig.h"
#include "shynet/utils/StringOp.h"
#include "shynet/utils/Stuff.h"
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
    using namespace frmpub;
    using namespace login;
    try {
        const char* file = "gameframe.ini";
        IniConfig& ini = Singleton<IniConfig>::instance(std::move(file));
        bool daemon = ini.get<bool, bool>("login", "daemon", false);
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
        int sid = ini.get<int, int>("login", "sid", 0);
        const char* pid_dir = "./pid/";
        if (access(pid_dir, F_OK) == -1) {
            mkdir(pid_dir, S_IRWXU);
        }
        std::string pidfile = StringOp::str_format("./%s/login_%d.pid", pid_dir, sid);
        Stuff::writepid(pidfile);

        Singleton<LuaEngine>::instance(std::make_shared<login::LuaWrapper>());
        Singleton<ThreadPool>::instance().start();

        //连接db服务器
        string dbstr = ini.get<const char*, string>("login", "db", "");
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

        //连接world服务器
        string worldstr = ini.get<const char*, string>("login", "world", "");
        auto worldlist = StringOp::split(worldstr, ",");
        if (worldlist.size() > 2 || worldlist.size() == 0) {
            THROW_EXCEPTION("world配置错误");
        }
        for (auto& item : worldlist) {
            std::string worldip = ini.get<const char*, string>(item, "ip", "");
            short worldport = ini.get<short, short>(item, "port", short(22000));
            Singleton<ConnectReactorMgr>::instance().add(
                std::shared_ptr<WorldConnector>(
                    new WorldConnector(std::shared_ptr<IPAddress>(
                        new IPAddress(worldip.c_str(), worldport)))));
        }

        LOG_DEBUG << "开启登录服服务器监听";
        std::string loginip = ini.get<const char*, std::string>("login", "ip", "127.0.0.1");
        short loginport = ini.get<short, short>("login", "port", short(23000));
        std::shared_ptr<IPAddress> loginaddr(new IPAddress(loginip.c_str(), loginport));
        std::shared_ptr<LoginServer> loginserver(new LoginServer(loginaddr));
        Singleton<ListenReactorMgr>::instance().add(loginserver);

        shared_ptr<EventBase> base(new EventBase());
        shared_ptr<StdinHandler> stdin(new StdinHandler(base, STDIN_FILENO));
        shared_ptr<SignalHandler> sigint(new SignalHandler(base));
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
