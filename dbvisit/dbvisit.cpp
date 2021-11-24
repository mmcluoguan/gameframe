#include "dbvisit/dbclientmgr.h"
#include "dbvisit/dbserver.h"
#include "dbvisit/luawrapper.h"
#include "frmpub/frmstdinhandler.h"
#include "frmpub/luafoldertask.h"
#include "shynet/events/eventhandler.h"
#include "shynet/io/stdinhandler.h"
#include "shynet/lua/luaengine.h"
#include "shynet/net/ipaddress.h"
#include "shynet/pool/mysqlpool.h"
#include "shynet/pool/threadpool.h"
#include "shynet/signal/signalhandler.h"
#include "shynet/utils/idworker.h"
#include "shynet/utils/iniconfig.h"
#include "shynet/utils/stringop.h"
#include "shynet/utils/stuff.h"
#include <google/protobuf/message.h>
#include <sw/redis++/redis++.h>
namespace redis = sw::redis;
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
    using namespace frmpub::protocc;
    using namespace dbvisit;
    using namespace redis;

    try {
        if (argc < 2) {
            THROW_EXCEPTION("没有配置参数");
        }
        g_confname = argv[1];
        const char* inifile = "gameframe.ini";
        IniConfig& ini = Singleton<IniConfig>::instance(std::move(inifile));
        bool daemon = ini.get<bool>(g_confname, "daemon");
        if (daemon) {
            Stuff::daemon();
        }
        int centerid = ini.get<int>(g_confname, "centerid");
        int workerid = ini.get<int>(g_confname, "workerid");
        Singleton<IdWorker>::instance(std::move(workerid), std::move(centerid));

        string ip = ini.get<string>(g_confname, "ip");
        short port = ini.get<short>(g_confname, "port");
        int sid = ini.get<int>(g_confname, "sid");
        string type = Basic::connectname(ServerType::DBVISIT);
        string name = ini.get<string>(g_confname, "name");

        string myuri = ini.get<string>(g_confname, "mysql_uri");
        size_t mysqlps = ini.get<size_t>(g_confname, "mysql_pool_size");
        mysqlx::SessionSettings myset(myuri);
        Singleton<MysqlPool>::instance(std::move(myset), std::move(mysqlps));

        ConnectionOptions connection_options;
        connection_options.host = ini.get<string>(g_confname, "redis_ip");
        LOG_DEBUG << connection_options.host;
        connection_options.port = ini.get<int>(g_confname, "redis_port");
        connection_options.db = ini.get<int>(g_confname, "redis_db");
        connection_options.password = ini.get<string>(g_confname, "redis_pwd");
        ConnectionPoolOptions pool_options;
        pool_options.size = ini.get<int>(g_confname, "redis_pool_size");
        Redis& redis = Singleton<Redis>::instance(std::move(connection_options), std::move(pool_options));

        string key = StringOp::str_format("%s_%d", type.c_str(), sid);
        bool ok = true;
        unordered_map<string, string> info;

        if (redis.exists(key) == 0) {
            redis.hmset(key,
                {
                    make_pair("ip", ip),
                    make_pair("port", to_string(port)),
                    make_pair("sid", to_string(sid)),
                    make_pair("type", type),
                    make_pair("name", name),
                });
        } else {
            redis.hgetall(key, std::inserter(info, info.begin()));
            if (ip != info["ip"] || to_string(port) != info["port"]) {
                ok = false;
            }
        }

        if (ok) {
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

            Singleton<LuaEngine>::instance(std::make_shared<dbvisit::LuaWrapper>());
            Singleton<ThreadPool>::instance().start();

            std::string luapath = ini.get<std::string>(g_confname, "luapath");
            std::vector<std::string> vectpath = StringOp::split(luapath, ";");
            for (string pstr : vectpath) {
                Singleton<ThreadPool>::get_instance().notifyTh().lock()->add(
                    std::make_shared<LuaFolderTask>(pstr, true));
            }

            shared_ptr<IPAddress> ipaddr(new IPAddress(ip.c_str(), port));
            shared_ptr<DbServer> dbserver(new DbServer(ipaddr));
            Singleton<ListenReactorMgr>::instance().add(dbserver);

            shared_ptr<EventBase> base(new EventBase());

            FrmStdinhandler* stdin = &Singleton<FrmStdinhandler>::instance(base);
            SignalHandler* sigint = &Singleton<SignalHandler>::instance(base);
            base->addevent(stdin, nullptr);
            base->addevent(sigint, nullptr);
            base->dispatch();
        } else {
            std::stringstream err;
            err << "已存在" << info["type"] << "_" << info["sid"] << " " << info["ip"] << ":" << info["port"];
            THROW_EXCEPTION(err.str());
        }
    } catch (const std::exception& err) {
        utils::Stuff::print_exception(err);
    }
    EventBase::cleanssl();
    EventBase::event_shutdown();
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
