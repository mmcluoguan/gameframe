#pragma once
#include "shynet/task/Task.h"

namespace shynet {
	namespace task {
		class TimerTask : public task::Task {
		public:
			explicit TimerTask(int timerid);
			~TimerTask();

			int run(thread::Thread* tif) override;
		private:
			int timerid_ = 0;
		};


	}
}
