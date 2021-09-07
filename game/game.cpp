#include <unistd.h>
#include "shynet/IniConfig.h"
#include "shynet/Logger.h"
#include "shynet/Utility.h"
#include "shynet/events/EventHandler.h"
#include "shynet/net/IPAddress.h"
#include "shynet/net/ConnectReactorMgr.h"
#include "shynet/pool/ThreadPool.h"
#include "shynet/lua/LuaEngine.h"
#include "shynet/IdWorker.h"
#include "frmpub/LuaFolderTask.h"
#include "game/ConnectorMgr.h"
#include "game/DbConnector.h"
#include "game/GameServer.h"
#include "game/StdinHandler.h"
#include "game/SignalHandler.h"
#include "game/LuaWrapper.h"

int main(int argc, char* argv[]) {
	using namespace std;
	using namespace shynet;
	using namespace shynet::events;
	using namespace shynet::pool;
	using namespace shynet::net;
	using namespace shynet::lua;
	using namespace frmpub;
	using namespace game;

	const char* file = "gameframe.ini";
	IniConfig& ini = Singleton<IniConfig>::instance(std::move(file));

	int centerid = ini.get<int, int>("dbvisit_game", "centerid", 1);
	int workerid = ini.get<int, int>("dbvisit_game", "workerid", 1);
	shynet::Singleton<IdWorker>::instance(std::move(workerid), std::move(centerid));

	bool daemon = ini.get<bool, bool>("dbvisit_game", "daemon", false);
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

	Singleton<LuaEngine>::instance(std::make_shared<game::LuaWrapper>());
	Singleton<ThreadPool>::instance().start();

	std::string luapath = ini.get<const char*, std::string>("game", "luapath", "");
	std::vector<std::string> vectpath = Utility::spilt(luapath, ";");
	for (string pstr : vectpath) {
		Singleton<ThreadPool>::get_instance().notifyTh().lock()->add(
			std::make_shared<LuaFolderTask>(pstr, true)
		);
	}

	string dbip = ini.get<const char*, string>("dbvisit_game", "ip", "127.0.0.1");
	short dbport = ini.get<short, short>("dbvisit_game", "port", short(21000));
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
