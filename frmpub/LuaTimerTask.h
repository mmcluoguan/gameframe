#pragma once
#include "shynet/net/TimerEvent.h"
#include "shynet/task/Task.h"
#include "shynet/utils/Singleton.h"
#include <unordered_set>

namespace frmpub {
	/*
	* lua计时器管理
	*/
	class LuaTimerMgr {
		class LuaTimerTask : public task::Task {
		public:
			LuaTimerTask(int timerid);

			int run(thread::Thread* tif) override;
		private:
			int timerid_;
		};

		class LuaTimer : public net::TimerEvent {
		public:
			LuaTimer(const struct timeval val, bool repeat);
			~LuaTimer();

			void timeout() override;
		private:
			bool repeat_ = true;
		};

		friend class shynet::utils::Singleton<LuaTimerMgr>;
		LuaTimerMgr();
	public:
		/// <summary>
		/// 添加计时器
		/// </summary>
		/// <param name="val">间隔</param>
		/// <param name="repeat">是否重复</param>
		/// <returns></returns>
		int add(const struct timeval val, bool repeat = true);
		void remove(int timerid);
	private:
		std::unordered_set<int> timerids_;
	};


}
