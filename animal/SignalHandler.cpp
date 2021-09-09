#include "animal/SignalHandler.h"
#include <signal.h>
#include <sw/redis++/redis++.h>
namespace redis = sw::redis;
#include <shynet/IniConfig.h>
#include <shynet/Singleton.h>
#include "frmpub/Basic.h"

namespace animal {
	SigIntHandler::SigIntHandler(std::shared_ptr<events::EventBase> base) :
		events::EventHandler(base, SIGINT, EV_SIGNAL | EV_PERSIST) {
		GOBJ++;
		LOG_TRACE << "SigIntHandler:" << GOBJ;
	}

	SigIntHandler::~SigIntHandler() {
		GOBJ--;
		LOG_TRACE << "~SigIntHandler:" << GOBJ;
	}

	void SigIntHandler::signal(int signal) {
		shynet::IniConfig& ini = shynet::Singleton<shynet::IniConfig>::get_instance();
		std::string type = frmpub::Basic::connectname(protocc::ServerType::DBVISIT);
		int sid = ini.get<int, int>("animal", "sid", 1);
		std::string key = shynet::Utility::str_format("%s_%d", type.c_str(), sid);
		redis::Redis& redis = shynet::Singleton<redis::Redis>::get_instance();
		try {
			redis.del(key);
		}
		catch (const redis::Error& err) {
			LOG_WARN << err.what();
		}

		struct timeval delay = { 2, 0 };
		LOG_INFO << "捕获到中断信号,程序将在2秒后安全退出";
		base()->loopexit(&delay);
	}
}
