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
#include "shynet/utils/Stringify_stl.h"
#include <atomic>
#include <cstddef>

void test() {
	shynet::utils::Databuffer<> dt;
	std::string origin = "abcde";
	std::string res = shynet::crypto::md5::sum(origin);
	assert(res == std::string("ab56b4d92b40713acc5af89985d4b786"));

	shynet::utils::SkipList<std::string, int> sk;
	sk.insert({ "a",100 });
	sk.insert({ "b",200 });
	sk.insert({ "c",150 });
	sk.insert({ "d",200 });
	sk.insert({ "f",150 });
	sk.insert({ "g",250 });
	sk.insert({ "h",210 });
	std::cout << sk.debug_string();
	auto it = sk.update("h", 230);
	sk.update(it.first, 330);
	std::cout << sk.debug_string();
	auto fit = sk.find("f");
	std::cout << "(" << fit->first << "," << fit->second << ") " << std::endl;
	sk.erase("d");
	std::cout << sk.debug_string();

	shynet::utils::SkipList<std::string, int> sk1(sk);
	std::cout << sk1.debug_string();
	shynet::utils::SkipList<std::string, int> sk2(std::move(sk1));
	std::cout << sk1.debug_string();
	std::cout << sk2.debug_string();
	shynet::utils::SkipList<std::string, int> sk3 = std::move(sk2);
	std::cout << sk2.debug_string();
	std::cout << sk3.debug_string();

	auto itff = sk.find(80, 200);
	for (; itff.first != itff.second; ++itff.first)
	{
		auto& pair = *itff.first;
		std::cout << "(" << pair.first << "," << pair.second << ") ";
	}
	std::cout << std::endl;

	auto rit = sk.rank_rang(2, 2);
	for (; rit.first != rit.second; ++rit.first)
	{
		auto& pair = *rit.first;
		std::cout << "(" << pair.first << "," << pair.second << ") ";
	}
	std::cout << std::endl;

	//sl.print();
	//shynet::utils::Lru<std::string, int> c(3);
	//c.put("chef", 1);
	//c.put("yoko", 2);
	//c.put("tom", 3);
	//c.put("jerry", 4); // 超过容器大小，淘汰最老的`chef`
	//bool exist;
	//int v;
	//exist = c.get("chef", &v);
	//assert(!exist);
	//exist = c.get("yoko", &v);
	//assert(exist && v == 2);
	//c.put("garfield", 5); // 超过容器大小，注意，由于`yoko`刚才读取时会更新热度，所以淘汰的是`tom`
	//exist = c.get("yoko", &v);
	//assert(exist && v == 2);
	//exist = c.get("tom", &v);
	//assert(!exist);

	////LOG_DEBUG << "hash code:" << shynet::utils::hash_val(v, exist);

	//std::byte bb{ 12 };
	//bb >>= 1;
	////LOG_DEBUG << std::to_integer<int>(bb);

	//std::shared_ptr<char[]> shccc(new char[10]{ 'a','1' });
	////LOG_DEBUG << shccc.get()[0];

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
		shared_ptr<SignalHandler> sigint(new SignalHandler(base));
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
