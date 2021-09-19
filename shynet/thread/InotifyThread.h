#ifndef SHYNET_THREAD_INOTIFYTHREAD_H
#define SHYNET_THREAD_INOTIFYTHREAD_H

#include "shynet/events/EventBuffer.h"
#include "shynet/task/NotifyTask.h"
#include "shynet/thread/Thread.h"
#include <mutex>

namespace shynet {
	namespace thread {
		/*
		* 文件监控线程
		*/
		class InotifyThread : public Thread {
		public:
			explicit InotifyThread(size_t index);
			~InotifyThread();

			int run() override;
			int stop() override;

			/*
			* 发生的事件
			*/
			void io_readcb();

			/*
			* 添加监控
			*/
			void add(std::shared_ptr<task::NotifyTask> task);

			/*
			* 移除监控
			*/
			void remove(std::shared_ptr<task::NotifyTask> task);

		private:
			std::mutex task_mutex_;
			std::unordered_map<int, std::shared_ptr<task::NotifyTask>> task_map_;
			int notifyfd_;
			std::shared_ptr<events::EventBase> base_ = nullptr;
			std::shared_ptr<events::EventBuffer> iobuf_ = nullptr;
		};
	}
}

#endif
