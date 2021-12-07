#include "client/consolehandler.h"
#include "client/gateconnector.h"
#include "shynet/events/eventhandler.h"
#include "shynet/lua/luaengine.h"
#include "shynet/net/connectreactormgr.h"
#include "shynet/net/ipaddress.h"
#include "shynet/pool/threadpool.h"
#include "shynet/signal/signalhandler.h"
#include "shynet/utils/iniconfig.h"
#include "shynet/utils/stuff.h"
#include <iomanip>
#include <unistd.h>

int g_gateconnect_id;

#include "shynet/crypto/md5.h"
#include "shynet/utils/databuffer.h"
#include "shynet/utils/hash.h"
#include "shynet/utils/lru.h"
#include "shynet/utils/skiplist.h"
#include "shynet/utils/stringify_stl.h"
#include <atomic>
#include <cstddef>
#include <dbg.h>
#include <tuple>
#include <unordered_map>
#include <visit_struct/visit_struct.hpp>

void test()
{
    shynet::utils::Databuffer<> dt;
    std::string origin = "abcde";
    std::string res = shynet::crypto::md5::sum(origin);
    assert(res == std::string("ab56b4d92b40713acc5af89985d4b786"));

    shynet::utils::SkipList<std::string, int> sk;
    sk.insert({ "a", 100 });
    sk.insert({ "b", 200 });
    sk.insert({ "c", 150 });
    sk.insert({ "d", 210 });
    sk.insert({ "f", 90 });
    sk.insert({ "g", 250 });
    sk.insert({ "h", 210 });
    //std::cout << sk.debug_string();
    auto it = sk.update("h", 230);
    //std::cout << sk.debug_string();
    sk.update(it.first, 250);
    std::cout << sk.debug_string();
    std::cout << sk << std::endl;
    //auto fit = sk.find("f");
    //std::cout << "(" << fit->first << "," << fit->second << "," << sk.pos_rank(fit) << ") " << std::endl;
    //sk.erase("d");
    //std::cout << sk.debug_string();
    //auto rank_pos = sk.rank_pos(5);
    //if (rank_pos != sk.end())
    //	std::cout << "(" << rank_pos->first << "," << rank_pos->second << ") " << std::endl;
    /*shynet::utils::SkipList<std::string, int> sk1(sk);
		std::cout << sk1.debug_string();
		shynet::utils::SkipList<std::string, int> sk2(std::move(sk1));
		std::cout << sk1.debug_string();
		std::cout << sk2.debug_string();
		shynet::utils::SkipList<std::string, int> sk3 = std::move(sk2);
		std::cout << sk2.debug_string();
		std::cout << sk3.debug_string();*/

    /*auto itff = sk.find(80, 200);
			for (; itff.first != itff.second; ++itff.first) {
				auto& pair = *itff.first;
				std::cout << "(" << pair.first << "," << pair.second << ") ";
			}
			std::cout << std::endl;*/

    auto rit = sk.rank_rang(2, 1000);
    for (; rit.first != rit.second; ++rit.first) {
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

void test1()
{
    dbg("====== primitive types");

    int test_int = 42;
    const float test_float = 3.14f;
    const bool test_bool = false;
    const char test_char = 'X';
    const uint64_t test_uint64_t = 12345678987654321;
    int* test_pointer = &test_int;
    const int* test_pointer_to_const = &test_int;
    int* test_pointer_null = nullptr;
    const int& test_ref_to_int = test_int;
    const char* test_c_string = "hello";
    const char test_c_chararray[] = "hello";
    const std::string test_string = "hello";

    dbg(test_int);
    dbg(test_float);
    dbg(test_bool);
    dbg(test_char);
    dbg(test_uint64_t);
    dbg(test_pointer);
    dbg(test_pointer_to_const);
    dbg(test_pointer_null);
    dbg(test_ref_to_int);
    dbg(test_c_string);
    dbg(test_c_chararray);
    dbg(test_string);

    dbg("====== r-values, literals, constants, etc.");

    dbg(42);
    dbg(3.14);
    dbg(false);
    dbg(12345678987654321);
    dbg(static_cast<void*>(nullptr));
    dbg("string literal");

    std::string message = "hello world";
    dbg(std::move(message));

    dbg(sizeof(int));

    dbg("====== expressions inside macros");

    dbg(9 + 33);
    dbg(test_string + " world");

    dbg("====== multiple arguments");

    dbg(test_int, (std::vector<int> { 2, 3, 4 }), test_string);

    dbg("====== containers");

    const std::vector<int> dummy_vec_int { 3, 2, 3 };
    dbg(dummy_vec_int);

    std::vector<int> dummy_vec_int_nonconst { 1, 2, 3 };
    dbg(dummy_vec_int_nonconst);

    const std::vector<int> dummy_vec_empty {};
    dbg(dummy_vec_empty);

    std::vector<char> vec_chars { 'h', 'e', 'l', 'l', 'o', '\x00', '\xFE' };
    dbg(vec_chars);

    std::vector<bool> vec_bools { true, true, false, false, false, true, false };
    dbg(vec_bools);

    dbg((std::vector<int> { 0, 1, 0, 1 }));

    const std::array<int, 2> dummy_array { { 0, 4 } };
    dbg(dummy_array);

    const std::list<int> dummy_list { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    dbg(dummy_list);

    std::vector<std::vector<int>> vec_of_vec_of_ints { { 1, 2 }, { 3, 4, 5 } };
    dbg(vec_of_vec_of_ints);

    std::vector<std::vector<std::vector<int>>> vec_of_vec_of_vec_of_ints {
        { { 1, 2 }, { 3, 4, 5 } }, { { 3 } }
    };
    dbg(vec_of_vec_of_vec_of_ints);

    int dummy_int_array[] = { 11, 22, 33 };
    dbg(dummy_int_array);

    dbg("====== integer formatting");
    dbg(dbg::hex(42));
    dbg(dbg::bin(0x00ff00ff));

    int8_t negative_five = -5;
    dbg(dbg::bin(negative_five));
    dbg(dbg::bin(static_cast<uint8_t>(negative_five)));

    dbg("====== std::tuple and std::pair");
    dbg((std::tuple<std::string, int, float> { "Hello", 7, 3.14f }));
    dbg((std::pair<std::string, int> { "Hello", 7 }));

#if DBG_MACRO_CXX_STANDARD >= 17
    dbg("====== Sum types");
    dbg(std::make_optional<bool>(false));
    dbg((std::variant<int, std::string> { "test" }));

    dbg("======= std::string_view");
    dbg(std::string_view { "test" });
#endif

    dbg("====== function name tests");

    class user_defined_class {
    public:
        user_defined_class() { }
        void some_method(int x) { dbg(x); }
    };
    user_defined_class {}.some_method(42);

    [](int x) {
        dbg("called from within a lambda!");
        return x;
    }(42);

    dbg("====== type names without a value");
    using IsSame = std::is_same<uint8_t, char>::type;

    struct user_defined_trivial_type {
        int32_t a;
        bool b;
    };

    dbg(dbg::type<IsSame>());
    dbg(dbg::type<int32_t>());
    dbg(dbg::type<const int32_t>());
    dbg(dbg::type<int32_t*>());
    dbg(dbg::type<int32_t&>());
    dbg(dbg::type<user_defined_trivial_type>());
    dbg(dbg::type<user_defined_class>());

    dbg("====== timestamp");
    dbg(dbg::time());
}

struct test_struct_one {
    int a;
    float b;
    std::string c;
};

VISITABLE_STRUCT(test_struct_one, a, b, c);

struct debug_printer {
    template <typename T>
    void operator()(const char* name, const T& t) const
    {
        std::cout << "  " << name << ": " << t << std::endl;
    }
};

template <typename T>
void debug_print(const T& t)
{
    std::cout << "{\n";
    visit_struct::apply_visitor(debug_printer {}, t);
    std::cout << "}" << std::endl;
}

using spair = std::pair<std::string, std::string>;

struct test_visitor_one {
    std::vector<spair> result;

    void operator()(const char* name, const std::string& s)
    {
        result.emplace_back(spair { std::string { name }, s });
    }

    template <typename T>
    void operator()(const char* name, const T& t)
    {
        result.emplace_back(spair { std::string { name }, std::to_string(t) });
    }
};

void test2()
{
    test_struct_one s { 5, 7.5f, "asdf" };
    debug_print(s);

    assert(visit_struct::field_count(s) == 3);
    assert(visit_struct::get<0>(s) == 5);
    assert(visit_struct::get<1>(s) == 7.5f);
    assert(visit_struct::get<2>(s) == "asdf");
    assert(visit_struct::get_name<0>(s) == std::string { "a" });
    assert(visit_struct::get_name<1>(s) == std::string { "b" });
    assert(visit_struct::get_name<2>(s) == std::string { "c" });
    assert(visit_struct::get_accessor<0>(s)(s) == visit_struct::get<0>(s));
    assert(visit_struct::get_accessor<1>(s)(s) == visit_struct::get<1>(s));
    assert(visit_struct::get_accessor<2>(s)(s) == visit_struct::get<2>(s));

    test_visitor_one vis1;
    visit_struct::apply_visitor(vis1, s);

    assert(vis1.result.size() == 3);
    assert(vis1.result[0].first == "a");
    assert(vis1.result[0].second == "5");
    assert(vis1.result[1].first == "b");
    assert(vis1.result[1].second == "7.500000");
    assert(vis1.result[2].first == "c");
    assert(vis1.result[2].second == "asdf");
}

int main(int argc, char* argv[])
{

    //test();
    //test1();
    //test2();
    //return 0;
    using namespace std;
    using namespace shynet;
    using namespace shynet::utils;
    using namespace shynet::events;
    using namespace shynet::pool;
    using namespace shynet::net;
    using namespace shynet::lua;
    using namespace shynet::signal;
    using namespace frmpub;
    using namespace client;
    try {
        const char* inifile = "gameframe.ini";
        IniConfig& ini = Singleton<IniConfig>::instance(std::move(inifile));
        stuff::create_coredump();
        Logger::set_loglevel(Logger::LogLevel::DEBUG);
        if (EventBase::usethread() == -1) {
            THROW_EXCEPTION("call usethread");
        }
        EventBase::initssl();

        Singleton<ThreadPool>::instance().start();

        string gateip = ini.get<string>("client", "gateip");
        short gateport = ini.get<short>("client", "gateport");
        shared_ptr<IPAddress> gateaddr(new IPAddress(gateip.c_str(), gateport));
        shared_ptr<GateConnector> gateconnect(new GateConnector(gateaddr));
        gateaddr.reset();
        g_gateconnect_id = Singleton<ConnectReactorMgr>::instance().add(gateconnect);
        gateconnect.reset();

        shared_ptr<EventBase> base(new EventBase());
        ConsoleHandler* stdin = &Singleton<ConsoleHandler>::instance(base);
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
