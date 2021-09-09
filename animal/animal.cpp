#include <unistd.h>
#include <unordered_map>
#include <google/protobuf/message.h>
#include <sw/redis++/redis++.h>
#include <curl/curl.h>
namespace redis = sw::redis;
#include "shynet/IniConfig.h"
#include "shynet/Logger.h"
#include "shynet/Utility.h"
#include "shynet/events/EventHandler.h"
#include "shynet/net/IPAddress.h"
#include "shynet/pool/ThreadPool.h"
#include "shynet/pool/MysqlPool.h"
#include "frmpub/Basic.h"
#include "frmpub/IdWorker.h"
#include "animal/AmServer.h"
#include "animal/StdinHandler.h"
#include "animal/SignalHandler.h"
#include "animal/AmClientMgr.h"

int main(int argc, char* argv[]) {
	using namespace std;
	using namespace shynet;
	using namespace shynet::events;
	using namespace shynet::pool;
	using namespace shynet::net;
	using namespace frmpub;
	using namespace frmpub::protocc;
	using namespace animal;
	using namespace redis;

	IniConfig& ini = Singleton<IniConfig>::instance(std::move(string("animal.ini").c_str()));
	string ip = ini.get<const char*, string>("animal", "ip", "127.0.0.1");
	short port = ini.get<short, short>("animal", "port", short(21000));
	int sid = ini.get<int, int>("animal", "sid", 1);
	bool daemon = ini.get<bool, bool>("animal", "daemon", false);
	string type = Basic::connectname(ServerType::ANIMAL);
	string name = ini.get<const char*, string>("animal", "name", "");

	shynet::Singleton<IdWorker>::instance(std::move(sid), std::move(sid));

	string myuri = ini.get<const char*, string>("mysql", "uri", "mysqlx://root:123456@127.0.0.1:33060/AnimalPlanet");
	size_t mysqlps = ini.get<size_t, size_t>("mysql", "pool_size", 3);
	mysqlx::SessionSettings myset(myuri);
	Singleton<MysqlPool>::instance(std::move(myset), std::move(mysqlps));

	ConnectionOptions connection_options;
	connection_options.host = ini.get<const char*, string>("redis", "ip", "127.0.0.1");
	connection_options.port = ini.get<int, int>("redis", "port", 1);
	connection_options.db = ini.get<int, int>("redis", "db", 0);
	connection_options.password = ini.get<const char*, string>("redis", "pwd", "Aninmal20200809!$");
	ConnectionPoolOptions pool_options;
	pool_options.size = ini.get<int, int>("redis", "pool_size", 3);
	Redis& redis = Singleton<Redis>::instance(std::move(connection_options), std::move(pool_options));

	string key = Utility::str_format("%s_%d", type.c_str(), sid);
	bool ok = true;
	unordered_map<string, string> info;
	try {
		if (redis.exists(key) == 0) {
			redis.hmset(key,
				{
					make_pair("ip",ip),
					make_pair("port",to_string(port)),
					make_pair("sid",to_string(sid)),
					make_pair("type",type),
					make_pair("name",name),
				});
		}
		else {
			redis.hgetall(key, std::inserter(info, info.begin()));
			if (ip != info["ip"] ||
				to_string(port) != info["port"]) {
				ok = false;
			}
		}
	}
	catch (const redis::Error& err) {
		LOG_WARN << err.what();
	}
	if (ok) {
		if (daemon) {
			Utility::daemon();
			Singleton<IniConfig>::instance(std::move(string("gameframe.ini").c_str()));
		}
		Utility::create_coredump();
		Logger::loglevel(Logger::LogLevel::DEBUG);
		if (EventBase::usethread() == -1) {
			LOG_ERROR << "call usethread";
		}
		EventBase::initssl();
		curl_global_init(CURL_GLOBAL_ALL);

		Singleton<ThreadPool>::instance().start();

		//加载配置
		try {
			MysqlPool& mysql = shynet::Singleton<MysqlPool>::get_instance();
			std::string sql = shynet::Utility::str_format("select id,value from config");
			mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
			for (auto item : sr.fetchAll()) {
				shynet::Singleton<AmClientMgr>::instance().config_data[item.get(0)] = item.get(1).operator std::string();;
			}
		}
		catch (const mysqlx::Error& err) {
			LOG_WARN << err.what();
			return 0;
		}

		shared_ptr<IPAddress> ipaddr(new IPAddress(ip.c_str(), port));
		shared_ptr<AmServer> amserver(new AmServer(ipaddr));
		Singleton<ListenReactorMgr>::instance().add(amserver);

		shared_ptr<EventBase> base(new EventBase());

		shared_ptr<StdinHandler> stdin(new StdinHandler(base, STDIN_FILENO));
		shared_ptr<SigIntHandler> sigint(new SigIntHandler(base));
		base->addevent(stdin, nullptr);
		base->addevent(sigint, nullptr);

		base->dispatch();
		EventBase::cleanssl();
		EventBase::event_shutdown();
		curl_global_cleanup();
	}
	else {
		LOG_WARN << "已存在" << info["type"] << "_" << info["sid"] << " " << info["ip"] << ":" << info["port"];
	}
	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}
