#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include "shynet/thread/Thread.h"
#include "shynet/kaguya/kaguya.hpp"
#include "shynet/task/Task.h"

namespace shynet {
	namespace thread {
		class LuaThread : public Thread {
		public:
			explicit LuaThread(size_t index);
			~LuaThread();

			int run() override;
			int stop() override;

			size_t addTask(std::shared_ptr<task::Task> tk);

			kaguya::State* luaState() const {
				return luaState_;
			}

		private:
			std::mutex tasks_mutex_;
			std::condition_variable tasks_condvar_;
			std::queue<std::shared_ptr<task::Task>> tasks_;
			bool stop_ = false;
			kaguya::State* luaState_ = nullptr;
		};
	}
}
