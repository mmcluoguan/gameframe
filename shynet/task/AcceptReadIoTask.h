#pragma once
#include "shynet/net/AcceptNewFd.h"
#include "shynet/task/Task.h"
#include "shynet/thread/Thread.h"

namespace shynet {
	namespace task {
		class AcceptReadIoTask : public task::Task {
		public:
			explicit AcceptReadIoTask(std::weak_ptr<net::AcceptNewFd> newfd);
			~AcceptReadIoTask();

			int run(thread::Thread* tif) override;
		private:
			std::weak_ptr<net::AcceptNewFd> newfd_;
		};
	}
}