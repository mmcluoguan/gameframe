#include "dbvisit/SignalHandler.h"
#include <signal.h>
#include <sw/redis++/redis++.h>
namespace redis = sw::redis;
#include <shynet/IniConfig.h>
#include <shynet/Singleton.h>
#include "frmpub/Basic.h"

namespace dbvisit
{
	SigIntHandler::SigIntHandler(std::shared_ptr<events::EventBase> base) :
		events::EventHandler(base, SIGINT, EV_SIGNAL | EV_PERSIST)
	{
	}

	SigIntHandler::~SigIntHandler()
	{
	}

	void SigIntHandler::signal(int signal)
	{
		shynet::IniConfig& ini = shynet::Singleton<shynet::IniConfig>::get_instance();
		std::string type = frmpub::Basic::connectname(protocc::ServerType::DBVISIT);
		int sid = ini.get<int, int>("dbvisit", "sid", 1);
		std::string key = shynet::Utility::str_format("%s_%d", type.c_str(), sid);
		redis::Redis& redis = shynet::Singleton<redis::Redis>::instance(std::string());
		try
		{
			redis.del(key);
		}
		catch (const std::exception& err) {
			LOG_WARN << err.what();
		}

		struct timeval delay = { 2, 0 };
		LOG_INFO << "捕获到中断信号,程序将在2秒后安全退出";
		base()->loopexit(&delay);
	}
}
