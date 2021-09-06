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
#include "gate/DbConnector.h"
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

	Singleton<LuaEngine>::instance(std::make_shared<gate::LuaWrapper>());
	Singleton<ThreadPool>::instance().start();

	string dbip = ini.get<const char*, string>("dbvisit", "ip", "127.0.0.1");
	short dbport = ini.get<short, short>("dbvisit", "port", short(21000));
	shared_ptr<IPAddress> dbaddr(new IPAddress(dbip.c_str(), dbport));
	Singleton<ConnectReactorMgr>::instance().add(
		shared_ptr<DbConnector>(
			new DbConnector(shared_ptr<IPAddress>(
				new IPAddress(dbip.c_str(), dbport)))));

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
