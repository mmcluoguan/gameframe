#pragma once
#include <mutex>
#include <queue>
#include "shynet/thread/Thread.h"
#include "shynet/task/Task.h"

namespace shynet {
	namespace thread {
		class WorkThread : public Thread {
		public:
			explicit WorkThread(size_t index);
			~WorkThread();

			int run() override;
			int stop() override;

			size_t addTask(std::shared_ptr<task::Task> tk);

		private:
			std::mutex tasks_mutex_;
			std::queue<std::shared_ptr<task::Task>> tasks_;
			bool stop_ = false;
		};
	}
}
