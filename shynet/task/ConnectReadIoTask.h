#ifndef SHYNET_TASK_CONNECTREADIOTASK_H
#define SHYNET_TASK_CONNECTREADIOTASK_H

#include "shynet/net/ConnectEvent.h"
#include "shynet/task/Task.h"
#include "shynet/thread/Thread.h"

namespace shynet {
	namespace task {
		class ConnectReadIoTask : public task::Task {
		public:
			explicit ConnectReadIoTask(std::shared_ptr<net::ConnectEvent> cntevent);
			~ConnectReadIoTask();

			int run(thread::Thread* tif) override;
		private:
			std::shared_ptr<net::ConnectEvent> cntevent_;
		};
	}
}

#endif
