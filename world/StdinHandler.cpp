#include "world/StdinHandler.h"
#include <cstring>
#include <unistd.h>
#include <tuple>
#include "shynet/utils/Logger.h"
#include "shynet/utils/StringOp.h"

namespace world {
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
			LOG_WARN << "call read";
		}
		else {
			typedef std::tuple<const char*, const char*, std::function<void(const char* od, int argc, char** argv, const char* optarg)>> item;
			item orders[] = {
				item("quit",":",bind(&StdinHandler::quit_order, this,
					std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4)),
			};

			char* order = shynet::utils::StringOp::trim(msg);
			char* argv[20] = { 0 };
<<<<<<< HEAD
			int argc = shynet::utils::StringOp::split(order, " ", argv, 20);
=======
			int argc = shynet::utils::StringOp::spilt(order, " ", argv, 20);
>>>>>>> 97f5d8ccc1392d6c54dfc663b535a803fe2f1f9e
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
}
