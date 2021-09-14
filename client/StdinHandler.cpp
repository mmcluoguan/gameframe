#include "client/StdinHandler.h"
#include <cstring>
#include <unistd.h>
#include <tuple>
#include "shynet/net/ConnectReactorMgr.h"
#include "shynet/utils/Singleton.h"
#include "shynet/utils/StringOp.h"
#include "shynet/utils/IniConfig.h"
#include "shynet/utils/Stuff.h"
#include "frmpub/protocc/client.pb.h"
#include "client/GateConnector.h"

extern int optind, opterr, optopt;
extern char* optarg;
extern int g_gateconnect_id;

namespace client {
	StdinHandler::StdinHandler(std::shared_ptr<events::EventBase> base, evutil_socket_t fd) :
		events::EventHandler(base, fd, EV_READ | EV_PERSIST) {
	}

	StdinHandler::~StdinHandler() {
	}

	void StdinHandler::input(int fd) {
		char msg[1024];
		memset(&msg, 0, sizeof(msg));
		ssize_t ret = read(fd, msg, sizeof(msg));
		if (ret < 0) {
			LOG_WARN << "call read";
		}
		else {
			typedef std::tuple<const char*, const char*, std::function<void(const char*, int, char**, const char*)>> item;
			item orders[] = {
				item("quit",":",bind(&StdinHandler::quit_order, this,
					std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4)),
				item("info",":",bind(&StdinHandler::info_order, this,
					std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4)),
				item("reconnect",":",bind(&StdinHandler::reconnect_order, this,
					std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4)),
				item("login",":p:",bind(&StdinHandler::login_order, this,
					std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4)),
			};

			char* order = shynet::utils::StringOp::trim(msg);
			char* argv[20] = { 0 };
			int argc = shynet::utils::StringOp::split(order, " ", argv, 20);
			if (argc > 0) {
				bool flag = false;
				for (const auto& it : orders) {
					if (strcmp(argv[0], std::get<0>(it)) == 0) {
						std::get<2>(it)(std::get<0>(it), argc, argv, std::get<1>(it));
						flag = true;
					}
				}
				if (flag == false) {

					LOG_WARN << "没有可执行的命令";
					LOG_INFO << "可执行的命令列表";
					for (const auto& it : orders) {
						LOG_INFO << "	<" << std::get<0>(it) << " " << &std::get<1>(it)[1] << ">";
					}
				}
			}
		}
	}

	void StdinHandler::quit_order(const char* od, int argc, char** argv, const char* optstr) {
		struct timeval delay = { 2, 0 };
		LOG_INFO << "捕获到一个退出命令,程序将在2秒后安全退出";
		base()->loopexit(&delay);
	}

	void StdinHandler::info_order(const char* od, int argc, char** argv, const char* optstr) {
		char path[PATH_MAX] = { 0 };
		char processname[NAME_MAX] = { 0 };
		shynet::utils::Stuff::executable_path(path, processname, sizeof(path));
		LOG_INFO_BASE << "程序名:" << processname;
		LOG_INFO_BASE << "使用线程数:" << shynet::utils::Stuff::num_of_threads();
		LOG_INFO_BASE << "进程id:" << getpid();
		shynet::utils::Stuff::mem_info mem;
		shynet::utils::Stuff::obtain_mem_info(&mem);
		LOG_INFO_BASE << "虚拟内存:" << mem.virt_kbytes << "kb";
		LOG_INFO_BASE << "常驻内存:" << mem.res_kbytes << "kb";
		LOG_INFO_BASE << "已运行时间:" << shynet::utils::Stuff::up_duration_seconds() << "s";
	}

	void StdinHandler::reconnect_order(const char* od, int argc, char** argv, const char* optstr) {
		std::shared_ptr<GateConnector> gate = std::dynamic_pointer_cast<GateConnector>(
			shynet::utils::Singleton<net::ConnectReactorMgr>::instance().find(g_gateconnect_id));
		if (gate != nullptr) {
			std::shared_ptr<GateConnector::DisConnectData> ptr = gate->disconnect_data();
			gate->close(net::ConnectEvent::CloseType::CLIENT_CLOSE);
			gate.reset();
			sleep(1);
			shynet::utils::IniConfig& ini = shynet::utils::Singleton<shynet::utils::IniConfig>::get_instance();
			std::string gateip = ini.get<const char*, std::string>("gate", "ip", "127.0.0.1");
			short gateport = ini.get<short, short>("gate", "port", short(25000));
			std::shared_ptr<net::IPAddress> gateaddr(new net::IPAddress(gateip.c_str(), gateport));
			std::shared_ptr<GateConnector> gateconnect(new GateConnector(gateaddr, ptr));
			gateaddr.reset();
			g_gateconnect_id = shynet::utils::Singleton<net::ConnectReactorMgr>::instance().add(gateconnect);
			gateconnect.reset();
		}
		else {
			LOG_WARN << "连接已经释放";
		}
	}

	void StdinHandler::login_order(const char* od, int argc, char** argv, const char* optstr) {
		int opt;
		optind = 1;
		char* platform = nullptr;
		while ((opt = getopt(argc, argv, optstr)) != -1) {
			switch (opt) {
			case 'p': platform = optarg; break;
			case ':':
				LOG_WARN << od << " 丢失参数 (-" << (char)optopt << ")";
				break;
			case '?':
				LOG_WARN << od << " 未知选项 (-" << (char)optopt << ")";
				break;
			default:
				LOG_SYSFATAL << "call getopt";
			}
		}
		if (optind < argc) {
			LOG_WARN << " 第一个不是选项的参数是" << argv[optind] << "在argv[" << optind << "]";
		}
		if (platform == nullptr) {
			LOG_INFO << "usage: " << od << " [-p platform]";
			return;
		}
		protocc::login_client_gate_c msg;
		msg.set_platform_key(platform);

		std::shared_ptr<GateConnector> gate = std::dynamic_pointer_cast<GateConnector>(
			shynet::utils::Singleton<net::ConnectReactorMgr>::instance().find(g_gateconnect_id));
		if (gate != nullptr) {
			gate->send_proto(protocc::LOGIN_CLIENT_GATE_C, &msg);
			LOG_DEBUG << "发送" << frmpub::Basic::msgname(protocc::LOGIN_CLIENT_GATE_C);
		}
		else {
			LOG_WARN << "连接已经释放";
		}
	}
}
