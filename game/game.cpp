#include "frmpub/frmstdinhandler.h"
#include "frmpub/luafoldertask.h"
#include "game/connectormgr.h"
#include "game/dbconnector.h"
#include "game/gameserver.h"
#include "game/luawrapper.h"
#include "shynet/events/eventhandler.h"
#include "shynet/io/stdinhandler.h"
#include "shynet/lua/luaengine.h"
#include "shynet/net/connectreactormgr.h"
#include "shynet/net/ipaddress.h"
#include "shynet/pool/threadpool.h"
#include "shynet/signal/signalhandler.h"
#include "shynet/utils/idworker.h"
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
    using namespace game;

    try {
        const char* inifile = "gameframe.ini";
        IniConfig& ini = Singleton<IniConfig>::instance(std::move(inifile));
        bool daemon = ini.get<bool>("game", "daemon");
        if (daemon) {
            Stuff::daemon();
        }
        Stuff::create_coredump();
        Logger::loglevel(Logger::LogLevel::DEBUG);
        if (EventBase::usethread() == -1) {
            THROW_EXCEPTION("call usethread");
        }
        EventBase::initssl();
        int sid = ini.get<int>("game", "sid");
        const char* pid_dir = "./pid/";
        if (access(pid_dir, F_OK) == -1) {
            mkdir(pid_dir, S_IRWXU);
        }
        std::string pidfile = StringOp::str_format("./%s/game_%d.pid", pid_dir, sid);
        Stuff::writepid(pidfile);

        Singleton<LuaEngine>::instance(std::make_shared<game::LuaWrapper>());
        Singleton<ThreadPool>::instance().start();

        std::string luapath = ini.get<std::string>("game", "luapath");
        std::vector<std::string> vectpath = StringOp::split(luapath, ";");
        for (string pstr : vectpath) {
            Singleton<ThreadPool>::get_instance().notifyTh().lock()->add(
                std::make_shared<LuaFolderTask>(pstr, true));
        }

        LOG_DEBUG << "开启游戏服服务器监听";
        std::string gameip = ini.get<std::string>("game", "ip");
        short gameport = ini.get<short>("game", "port");
        std::shared_ptr<IPAddress> gameaddr(new IPAddress(gameip.c_str(), gameport));
        std::shared_ptr<GameServer> gameserver(new GameServer(gameaddr));
        Singleton<ListenReactorMgr>::instance().add(gameserver);

        //连接db服务器
        string dbstr = ini.get<string>("game", "db");
        auto dblist = StringOp::split(dbstr, ",");
        if (dblist.size() > 2 || dblist.size() == 0) {
            THROW_EXCEPTION("db配置错误");
        }
        for (auto& item : dblist) {

            int centerid = ini.get<int>(item, "centerid");
            int workerid = ini.get<int>(item, "workerid");
            Singleton<IdWorker>::instance(std::move(workerid), std::move(centerid));

            string dbip = ini.get<string>(item, "ip");
            short dbport = ini.get<short>(item, "port");
            shared_ptr<IPAddress> dbaddr(new IPAddress(dbip.c_str(), dbport));
            Singleton<ConnectReactorMgr>::instance().add(
                shared_ptr<DbConnector>(
                    new DbConnector(shared_ptr<IPAddress>(
                        new IPAddress(dbip.c_str(), dbport)))));
        }

        //连接world服务器
        string worldstr = ini.get<string>("game", "world");
        auto worldlist = StringOp::split(worldstr, ",");
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
        utils::Stuff::print_exception(err);
    }
    EventBase::cleanssl();
    EventBase::event_shutdown();
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
