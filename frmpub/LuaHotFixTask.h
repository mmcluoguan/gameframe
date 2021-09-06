#pragma once
#include "shynet/task/Task.h"
#include "shynet/thread/Thread.h"

namespace frmpub {
	/*
	* lua热更新任务
	*/
	class LuaHotFixTask : public shynet::task::Task {
	public:
		LuaHotFixTask(std::string filepath);
		~LuaHotFixTask();

		virtual int run(thread::Thread* tif);
	private:
		std::string filepath_;
	};
}
