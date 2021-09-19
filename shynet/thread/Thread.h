#pragma once
#include <shynet/Basic.h>
#include <thread>

namespace shynet {
	namespace thread {
		class Thread : public Nocopy {
		public:
			enum class ThreadType {
				GENERAL,
				TIMER,
				ACCEPT,
				CONNECT,
				LISTEN,
				LUA,
				INOTIFY,
				NUM_THREAD_TYPE,
			};

			/// <summary>
			/// 线程
			/// </summary>
			/// <param name="t">类型</param>
			/// <param name="index">线程池中的索引</param>
			Thread(ThreadType t, size_t index);
			~Thread();

			ThreadType type() const;
			void set_type(ThreadType v);
			size_t index() const;
			void set_index(size_t v);
			std::shared_ptr<std::thread> thread() const {
				return thread_;
			}

			std::shared_ptr<std::thread> start();
			virtual int run() = 0;
			virtual int stop() = 0;
		private:
			ThreadType type_ = ThreadType::GENERAL;
			std::shared_ptr<std::thread> thread_ = nullptr;
			/*
			* 线程创建索引
			*/
			size_t index_ = -1;
		};
	}
}
