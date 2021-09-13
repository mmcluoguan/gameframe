#include "dbvisit/SignalHandler.h"
#include <signal.h>
#include <sw/redis++/redis++.h>
namespace redis = sw::redis;
#include "shynet/utils/IniConfig.h"
#include "shynet/utils/Singleton.h"
#include "shynet/utils/StringOp.h"
#include "frmpub/protocc/common.pb.h"
#include "frmpub/Basic.h"

//配置参数
extern const char* g_confname;

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
		shynet::utils::IniConfig& ini = shynet::utils::Singleton<shynet::utils::IniConfig>::get_instance();
		std::string type = frmpub::Basic::connectname(protocc::ServerType::DBVISIT);
		int sid = ini.get<int, int>(g_confname, "sid", 1);
		std::string key = shynet::utils::StringOp::str_format("%s_%d", type.c_str(), sid);
		redis::Redis& redis = shynet::utils::Singleton<redis::Redis>::instance(std::string());
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
