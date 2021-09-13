#include "shynet/task/NotifyTask.h"
#include "shynet/net/TimerReactorMgr.h"

namespace shynet {
	namespace task {

		NotifyTask::NotifyTask(std::string path, bool ischild, uint32_t mask) {
			path_ = path;
			ischild_ = ischild;
			mask_ = mask;
		}
		NotifyTask::~NotifyTask() {
		}
	}
}
