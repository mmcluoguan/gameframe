#include "shynet/thread/Thread.h"

namespace shynet {
	namespace thread {
		Thread::Thread(ThreadType t, size_t index) {
			type_ = t;
			index_ = index;
		}
		Thread::~Thread() {
		}
		Thread::ThreadType Thread::type() const {
			return type_;
		}
		void Thread::type(ThreadType v) {
			type_ = v;
		}
		size_t Thread::index() const {
			return index_;
		}
		void Thread::index(size_t v) {
			index_ = v;
		}
		std::shared_ptr<std::thread> Thread::start()
		{
			thread_ = std::make_shared<std::thread>(&Thread::run, this);
			return thread_;
		}
	}
}
