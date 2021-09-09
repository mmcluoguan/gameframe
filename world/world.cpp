#include <unistd.h>
#include "shynet/IniConfig.h"
#include "shynet/events/EventHandler.h"
#include "shynet/net/IPAddress.h"
#include "shynet/net/ConnectReactorMgr.h"
#include "shynet/pool/ThreadPool.h"
#include "shynet/lua/LuaEngine.h"
#include "world/DbConnector.h"
#include "world/WorldServer.h"
#include "world/SignalHandler.h"
#include "world/StdinHandler.h"
#include "world/LuaWrapper.h"
#include "world/HttpServer.h"

//配置参数
const char* g_confname;

int main(int argc, char* argv[]) {
	using namespace std;
	using namespace shynet;
	using namespace shynet::events;
	using namespace shynet::pool;
	using namespace shynet::net;
	using namespace shynet::lua;
	using namespace frmpub;
	using namespace world;

	if (argc < 2)
	{
		LOG_ERROR << "没有配置参数";
	}
	g_confname = argv[1];

	const char* file = "gameframe.ini";
	IniConfig& ini = Singleton<IniConfig>::instance(std::move(file));
	bool daemon = ini.get<bool, bool>(g_confname, "daemon", false);
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

	Singleton<LuaEngine>::instance(std::make_shared<world::LuaWrapper>());
	Singleton<ThreadPool>::instance().start();

	//连接db服务器
	string dbstr = ini.get<const char*, string>(g_confname, "db", "");
	auto dblist = Utility::spilt(dbstr, ",");
	if (dblist.size() > 2 || dblist.size() == 0){
		LOG_ERROR << "db配置错误:" << dbstr;
	}
	for (auto& item : dblist)
	{
		string dbip = ini.get<const char*, string>(item, "ip", "");
		short dbport = ini.get<short, short>(item, "port", short(21000));
		shared_ptr<IPAddress> dbaddr(new IPAddress(dbip.c_str(), dbport));
		Singleton<ConnectReactorMgr>::instance().add(
			shared_ptr<DbConnector>(
				new DbConnector(shared_ptr<IPAddress>(
					new IPAddress(dbip.c_str(), dbport)))));
	}

	LOG_DEBUG << "开启世界服务器监听";
	std::string worldip = ini.get<const char*, std::string>(g_confname, "ip", "127.0.0.1");
	short worldport = ini.get<short, short>(g_confname, "port", short(22000));
	std::shared_ptr<IPAddress> worldaddr(new IPAddress(worldip.c_str(), worldport));
	std::shared_ptr<WorldServer> worldserver(new WorldServer(worldaddr));
	shynet::Singleton<ListenReactorMgr>::instance().add(worldserver);

	LOG_DEBUG << "开启http后台服务器监听";
	std::string httpip = ini.get<const char*, std::string>(g_confname, "http_ip", "127.0.0.1");
	short httpport = ini.get<short, short>(g_confname, "http_port", short(26000));
	std::shared_ptr<IPAddress> httpaddr(new IPAddress(httpip.c_str(), httpport));
	std::shared_ptr<HttpServer> httpserver(new HttpServer(httpaddr));
	shynet::Singleton<ListenReactorMgr>::instance().add(httpserver);

	shared_ptr<EventBase> base(new EventBase());
	shared_ptr<StdinHandler> stdin(new StdinHandler(base, STDIN_FILENO));
	shared_ptr<SigIntHandler> sigint(new SigIntHandler(base));
	base->addevent(stdin, nullptr);
	base->addevent(sigint, nullptr);

	base->dispatch();
	EventBase::cleanssl();
	EventBase::event_shutdown();
	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}
