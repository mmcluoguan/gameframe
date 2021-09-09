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
#include "shynet/net/ConnectReactorMgr.h"
#include "shynet/pool/ThreadPool.h"
#include "shynet/pool/MysqlPool.h"
#include "frmpub/Basic.h"
#include "frmpub/IdWorker.h"
#include "backend/BkServer.h"
#include "backend/StdinHandler.h"
#include "backend/SignalHandler.h"
#include "backend/BkClientMgr.h"
#include "backend/AmConnector.h"

int main(int argc, char* argv[]) {
	using namespace std;
	using namespace shynet;
	using namespace shynet::events;
	using namespace shynet::pool;
	using namespace shynet::net;
	using namespace frmpub;
	using namespace frmpub::protocc;
	using namespace backend;
	using namespace redis;

	IniConfig& ini = Singleton<IniConfig>::instance(std::move(string("backend.ini").c_str()));
	string ip = ini.get<const char*, string>("backend", "ip", "127.0.0.1");
	short port = ini.get<short, short>("backend", "port", short(22000));
	int sid = ini.get<int, int>("backend", "sid", 1);
	bool daemon = ini.get<bool, bool>("backend", "daemon", false);
	string type = Basic::connectname(ServerType::BACKEND);
	string name = ini.get<const char*, string>("backend", "name", "");

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
			Singleton<IniConfig>::instance(std::move(string("backend.ini").c_str()));
		}
		Utility::create_coredump();
		Logger::loglevel(Logger::LogLevel::DEBUG);
		if (EventBase::usethread() == -1) {
			LOG_ERROR << "call usethread";
		}
		EventBase::initssl();
		curl_global_init(CURL_GLOBAL_ALL);

		Singleton<ThreadPool>::instance().start();

		shared_ptr<IPAddress> ipaddr(new IPAddress(ip.c_str(), port));
		shared_ptr<BkServer> regserver(new BkServer(ipaddr));
		Singleton<ListenReactorMgr>::instance().add(regserver);

		IniConfig& ini = Singleton<IniConfig>::instance(std::move(string("backend.ini").c_str()));
		string amip = ini.get<const char*, string>("animal", "ip", "127.0.0.1");
		short amport = ini.get<short, short>("animal", "port", short(21000));
		shared_ptr<IPAddress> dbaddr(new IPAddress(amip.c_str(), amport));
		Singleton<ConnectReactorMgr>::instance().add(
			shared_ptr<AmConnector>(
				new AmConnector(shared_ptr<IPAddress>(
					new IPAddress(amip.c_str(), amport)))));

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
