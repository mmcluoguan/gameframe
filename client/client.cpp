#include "client/GateConnector.h"
#include "client/StdinHandler.h"
#include "client/SignalHandler.h"
#include "shynet/events/EventHandler.h"
#include "shynet/lua/LuaEngine.h"
#include "shynet/net/IPAddress.h"
#include "shynet/net/ConnectReactorMgr.h"
#include "shynet/pool/ThreadPool.h"
#include "shynet/utils/IniConfig.h"
#include "shynet/utils/Stuff.h"
#include <unistd.h>
#include <iomanip>

int g_gateconnect_id;

#include "shynet/crypto/Md5.h"
#include "shynet/utils/Databuffer.h"
#include "shynet/utils/Hash.h"
#include "shynet/utils/Lru.h"
#include "shynet/utils/SkipList.h"
#include <cstddef>

void test() {
	shynet::utils::Databuffer<> dt;
	std::string origin = "abcde";
	std::string res = shynet::crypto::md5::sum(origin);
	assert(res == std::string("ab56b4d92b40713acc5af89985d4b786"));

	std::vector<int> aa;
	int max = 10;
	for (int i = 0; i < max; i++) {
		aa.push_back(5);//shynet::utils::Stuff::random(1, 5));
	}
	shynet::utils::SkipList<int, int> sl;
	for (int i = 0; i < max; i++) {
		int a = aa[i];
		sl.insert({ a,i });
	}
	//sl.print();
	shynet::utils::Lru<std::string, int> c(3);
	c.put("chef", 1);
	c.put("yoko", 2);
	c.put("tom", 3);
	c.put("jerry", 4); // 超过容器大小，淘汰最老的`chef`
	bool exist;
	int v;
	exist = c.get("chef", &v);
	assert(!exist);
	exist = c.get("yoko", &v);
	assert(exist && v == 2);
	c.put("garfield", 5); // 超过容器大小，注意，由于`yoko`刚才读取时会更新热度，所以淘汰的是`tom`
	exist = c.get("yoko", &v);
	assert(exist && v == 2);
	exist = c.get("tom", &v);
	assert(!exist);

	LOG_DEBUG << "hash code:" << shynet::utils::hash_val(v, exist);

	std::byte bb{ 12 };
	bb >>= 1;
	LOG_DEBUG << std::to_integer<int>(bb);

	std::shared_ptr<char[]> shccc(new char[10]{ 'a','1' });
	LOG_DEBUG << shccc.get()[0];
}

int main(int argc, char* argv[]) {

	test();
	return 0;
	using namespace std;
	using namespace shynet;
	using namespace shynet::utils;
	using namespace shynet::events;
	using namespace shynet::pool;
	using namespace shynet::net;
	using namespace shynet::lua;
	using namespace frmpub;
	using namespace client;
	try {
		const char* file = "gameframe.ini";
		IniConfig& ini = Singleton<IniConfig>::instance(std::move(file));
		bool daemon = ini.get<bool, bool>("register", "daemon", false);
		if (daemon) {
			Stuff::daemon();
			Singleton<IniConfig>::instance(std::move(string("gameframe.ini").c_str()));
		}

		Stuff::create_coredump();
		Logger::loglevel(Logger::LogLevel::DEBUG);
		if (EventBase::usethread() == -1) {
			THROW_EXCEPTION("call usethread");
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
	}
	catch (const std::exception& err) {
		utils::Stuff::print_exception(err);
	}
	EventBase::cleanssl();
	EventBase::event_shutdown();
	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}
