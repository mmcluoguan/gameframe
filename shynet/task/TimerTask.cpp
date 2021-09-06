#include "shynet/task/TimerTask.h"
#include "shynet/net/TimerReactorMgr.h"

namespace shynet {
	namespace task {

		TimerTask::TimerTask(int timerid) {
			timerid_ = timerid;
		}
		TimerTask::~TimerTask() {
		}

		int TimerTask::run(thread::Thread* tif) {
			std::shared_ptr<net::TimerEvent> tv = Singleton<net::TimerReactorMgr>::instance().find(timerid_);
			if (tv != nullptr) {
				tv->timeout();
			}
			return 0;
		}
	}
}
