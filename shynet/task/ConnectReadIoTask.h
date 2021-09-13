#pragma once
#include "shynet/thread/Thread.h"
#include "shynet/net/ConnectEvent.h"
#include "shynet/task/Task.h"

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
