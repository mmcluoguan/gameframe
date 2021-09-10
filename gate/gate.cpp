#include <unistd.h>
#include "shynet/IniConfig.h"
#include "shynet/Logger.h"
#include "shynet/Utility.h"
#include "shynet/events/EventHandler.h"
#include "shynet/net/IPAddress.h"
#include "shynet/net/ConnectReactorMgr.h"
#include "shynet/pool/ThreadPool.h"
#include "shynet/lua/LuaEngine.h"
#include "gate/ConnectorMgr.h"
#include "gate/GateClientMgr.h"
#include "gate/GateServer.h"
#include "gate/SignalHandler.h"
#include "gate/StdinHandler.h"
#include "gate/LuaWrapper.h"

int main(int argc, char* argv[]) {
	using namespace std;
	using namespace shynet;
	using namespace shynet::events;
	using namespace shynet::pool;
	using namespace shynet::net;
	using namespace shynet::lua;
	using namespace frmpub;
	using namespace gate;

	const char* file = "gameframe.ini";
	IniConfig& ini = Singleton<IniConfig>::instance(std::move(file));
	bool daemon = ini.get<bool, bool>("gate", "daemon", false);
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
	int sid = ini.get<int, int>("gate", "sid", 0);
	std::string pidfile = Utility::str_format("./gate_%d.pid", sid);
	Utility::writepid(pidfile);

	Singleton<LuaEngine>::instance(std::make_shared<gate::LuaWrapper>());
	Singleton<ThreadPool>::instance().start();

	LOG_DEBUG << "开启网关服服务器监听";
	std::string gateip = ini.get<const char*, std::string>("gate", "ip", "127.0.0.1");
	short gateport = ini.get<short, short>("gate", "port", short(25000));
	std::shared_ptr<IPAddress> gateaddr(new IPAddress(gateip.c_str(), gateport));
	std::shared_ptr<GateServer> gateserver(new GateServer(gateaddr));
	shynet::Singleton<ListenReactorMgr>::instance().add(gateserver);

	//连接world服务器
	string worldstr = ini.get<const char*, string>("gate", "world", "");
	auto worldlist = Utility::spilt(worldstr, ",");
	if (worldlist.size() > 2 || worldlist.size() == 0) {
		LOG_ERROR << "world配置错误:" << worldstr;
	}
	for (auto& item : worldlist)
	{
		std::string worldip = ini.get<const char*, string>(item, "ip", "");
		short worldport = ini.get<short, short>(item, "port", short(22000));
		shynet::Singleton<ConnectReactorMgr>::instance().add(
			std::shared_ptr<WorldConnector>(
				new WorldConnector(std::shared_ptr<IPAddress>(
					new IPAddress(worldip.c_str(), worldport)))));
	}

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
