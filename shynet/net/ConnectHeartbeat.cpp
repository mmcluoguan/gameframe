#include "shynet/net/ConnectHeartbeat.h"
#include "shynet/net/TimerReactorMgr.h"
#include "shynet/net/ConnectEvent.h"

namespace shynet
{

	namespace net
	{
		ConnectHeartbeat::ConnectHeartbeat(std::weak_ptr<ConnectEvent> cnv, const struct timeval val) :
			TimerEvent(val, EV_TIMEOUT)
		{
			cnv_ = cnv;
		}
		ConnectHeartbeat::~ConnectHeartbeat()
		{
		}
		void ConnectHeartbeat::timeout()
		{
			utils::Singleton<TimerReactorMgr>::instance().remove(timerid());
			std::shared_ptr<ConnectEvent> cnv = cnv_.lock();
			if (cnv != nullptr)
			{
				cnv->timerout();
			}
		}


	}
}
