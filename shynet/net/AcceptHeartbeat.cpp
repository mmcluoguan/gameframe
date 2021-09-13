#include "shynet/net/AcceptHeartbeat.h"
#include "shynet/net/AcceptNewFd.h"

namespace shynet
{
	namespace net
	{
		AcceptHeartbeat::AcceptHeartbeat(std::weak_ptr<AcceptNewFd> newfd, const struct timeval val) :
			TimerEvent(val, EV_TIMEOUT)
		{
			newfd_ = newfd;
		}
		AcceptHeartbeat::~AcceptHeartbeat()
		{
		}
		void AcceptHeartbeat::timeout()
		{
			utils::Singleton<TimerReactorMgr>::instance().remove(timerid());
			std::shared_ptr<AcceptNewFd> newfd = newfd_.lock();
			if (newfd != nullptr)
			{
				newfd->timerout();
			}
		}
	}
}
