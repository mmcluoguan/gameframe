#include "gate/StdinHandler.h"
#include "shynet/utils/Logger.h"
#include "shynet/utils/StringOp.h"
#include "shynet/utils/Stuff.h"
#include <limits.h>
#include <unistd.h>
#include <tuple>
#include <cstring>

namespace gate {
	StdinHandler::StdinHandler(std::shared_ptr<events::EventBase> base, evutil_socket_t fd) :
		events::EventHandler(base, fd, EV_READ | EV_PERSIST) {
	}

	StdinHandler::~StdinHandler() {
	}


	extern int optind, opterr, optopt;

	void StdinHandler::input(int fd) {
		char msg[1024];
		memset(&msg, 0, sizeof(msg));
		ssize_t ret = read(fd, msg, sizeof(msg));
		if (ret < 0) {
			THROW_EXCEPTION("call read");
		}
		else {
			using item = std::tuple<const char*, const char*, std::function<void(const char* od, int argc, char** argv, const char* optarg)>>;
			item orders[] = {
				item("quit",":",bind(&StdinHandler::quit_order, this,
					std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4)),
				item("info",":",bind(&StdinHandler::info_order, this,
					std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4)),
			};

			char* order = shynet::utils::StringOp::trim(msg);
			char* argv[20] = { 0 };
			int argc = shynet::utils::StringOp::split(order, " ", argv, 20);
			if (argc > 0) {
				bool flag = false;
				for (auto&& [item0, item1, itemfun] : orders) {
					if (strcmp(argv[0], item0) == 0) {
						itemfun(item0, argc, argv, item1);
						flag = true;
					}
				}
				if (flag == false) {
					LOG_INFO << "没有可执行的命令";
					LOG_INFO << "可执行的命令列表";
					for (const auto& it : orders) {
						LOG_INFO << "	<" << std::get<0>(it) << ">";
					}
				}
			}
		}
	}

	void StdinHandler::quit_order(const char* od, int argc, char** argv, const char* optarg) {
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
}
