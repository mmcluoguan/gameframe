#include "3rd/sw/redis++/redis++.h"
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
#include "shynet/utils/filepathop.h"
#include "shynet/utils/idworker.h"
#include "shynet/utils/iniconfig.h"
#include "shynet/utils/stringop.h"
#include "shynet/utils/stuff.h"
#include <google/protobuf/message.h>
namespace redis = sw::redis;
#include <csignal>
#include <sys/stat.h>
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
    using namespace frmpub::protocc;
    using namespace dbvisit;
    using namespace redis;

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
        Singleton<IdWorker>::instance(std::move(workerid), std::move(centerid));

        string ip = ini.get<string>(g_conf_node, "ip");
        short port = ini.get<short>(g_conf_node, "port");
        int sid = ini.get<int>(g_conf_node, "sid");
        string type = Basic::connectname(ServerType::DBVISIT);
        string name = ini.get<string>(g_conf_node, "name");

        string myuri = ini.get<string>(g_conf_node, "mysql_uri");
        size_t mysqlps = ini.get<size_t>(g_conf_node, "mysql_pool_size");
        mysqlx::SessionSettings myset(myuri);
        Singleton<MysqlPool>::instance(std::move(myset), std::move(mysqlps));

        ConnectionOptions connection_options;
        connection_options.host = ini.get<string>(g_conf_node, "redis_ip");
        connection_options.port = ini.get<int>(g_conf_node, "redis_port");
        connection_options.db = ini.get<int>(g_conf_node, "redis_db");
        connection_options.password = ini.get<string>(g_conf_node, "redis_pwd");
        ConnectionPoolOptions pool_options;
        pool_options.size = ini.get<int>(g_conf_node, "redis_pool_size");
        Redis& redis = Singleton<Redis>::instance(std::move(connection_options), std::move(pool_options));

        string key = stringop::str_format("%s_%d", type.c_str(), sid);
        unordered_map<string, string> info;

        redis.hmset(key,
            {
                make_pair("ip", ip),
                make_pair("port", to_string(port)),
                make_pair("sid", to_string(sid)),
                make_pair("type", type),
                make_pair("name", name),
            });

        if (EventBase::usethread() == -1) {
            THROW_EXCEPTION("call usethread");
        }
        EventBase::initssl();

        Singleton<LuaEngine>::instance(std::make_shared<dbvisit::LuaWrapper>());
        Singleton<ThreadPool>::instance().start();

        std::string luapath = ini.get<std::string>(g_conf_node, "luapath");
        std::vector<std::string> vectpath = stringop::split(luapath, ";");
        for (string pstr : vectpath) {
            Singleton<ThreadPool>::get_instance().notifyTh().lock()->add(
                std::make_shared<LuaFolderTask>(pstr, true));
        }

        shared_ptr<IPAddress> ipaddr(new IPAddress(ip.c_str(), port));
        shared_ptr<DbServer> dbserver(new DbServer(ipaddr));
        Singleton<ListenReactorMgr>::instance().add(dbserver);

        const char* pid_dir = "./pid/";
        filepathop::mkdir_recursive(pid_dir);
        std::string pidfile = stringop::str_format("./%s/%s.pid", pid_dir, g_conf_node);
        stuff::writepid(pidfile);

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
