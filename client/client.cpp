#include <unistd.h>
#include "shynet/IniConfig.h"
#include "shynet/events/EventHandler.h"
#include "shynet/net/IPAddress.h"
#include "shynet/net/ConnectReactorMgr.h"
#include "shynet/pool/ThreadPool.h"
#include "shynet/lua/LuaEngine.h"
#include "client/GateConnector.h"
#include "client/SignalHandler.h"
#include "client/StdinHandler.h"

//#include "notifymgr.h"

//void test() {
//	shynet::Singleton<utils::NotifyMgr>::instance().start();
//	shynet::Singleton<utils::NotifyMgr>::instance().add("proto", true);
//}

int g_gateconnect_id;

int main(int argc, char* argv[]) {

	//test();

	using namespace std;
	using namespace shynet;
	using namespace shynet::events;
	using namespace shynet::pool;
	using namespace shynet::net;
	using namespace shynet::lua;
	using namespace frmpub;
	using namespace client;

	const char* file = "gameframe.ini";
	IniConfig& ini = Singleton<IniConfig>::instance(std::move(file));
	bool daemon = ini.get<bool, bool>("register", "daemon", false);
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

	Singleton<ThreadPool>::instance().start();

	string gateip = ini.get<const char*, string>("gate", "ip", "127.0.0.1");
	short gateport = ini.get<short, short>("gate", "port", short(25000));
	shared_ptr<IPAddress> gateaddr(new IPAddress(gateip.c_str(), gateport));
	shared_ptr<GateConnector> gateconnect(new GateConnector(gateaddr));
	gateaddr.reset();
	g_gateconnect_id = Singleton<ConnectReactorMgr>::instance().add(gateconnect);
	gateconnect.reset();

	shared_ptr<EventBase> base(new EventBase());
	shared_ptr<StdinHandler> stdin(new StdinHandler(base, STDIN_FILENO));
	shared_ptr<SigIntHandler> sigint(new SigIntHandler(base));
	base->addevent(stdin, nullptr);
	base->addevent(sigint, nullptr);
	base->dispatch();
	
	//shynet::Singleton<utils::NotifyMgr>::instance().stop();
	
	EventBase::cleanssl();
	EventBase::event_shutdown();
	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}
