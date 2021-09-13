#include "gate/SignalHandler.h"
#include <signal.h>
#include "shynet/utils/Logger.h"

namespace gate
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
		struct timeval delay = { 2, 0 };
		LOG_INFO << "捕获到中断信号,程序将在2秒后安全退出";
		base()->loopexit(&delay);
	}
}
