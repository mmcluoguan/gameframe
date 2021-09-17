#include "shynet/net/TimerEvent.h"
#include "shynet/pool/ThreadPool.h"
#include "shynet/task/TimerTask.h"

namespace shynet {

	namespace net {
		TimerEvent::TimerEvent(const struct timeval val, short what) :
			EventHandler(), val_(val) {
			what_ = what;
		}
		TimerEvent::~TimerEvent() {
		}

		const timeval& TimerEvent::val() const {
			return val_;
		}

		void TimerEvent::set_val(const timeval& t) {
			if (event() != nullptr) {
				val_.tv_sec = t.tv_sec;
				val_.tv_usec = t.tv_usec;
				event_add(event(), &val_);
			}
		}

		short TimerEvent::what() const {
			return what_;
		}

		int TimerEvent::timerid() const {
			return timerid_;
		}

		void TimerEvent::timerid(int timerid) {
			timerid_ = timerid;
		}

		void TimerEvent::timeout(int fd) {
			std::shared_ptr<task::TimerTask> ttk = std::make_shared<task::TimerTask>(timerid_);
			utils::Singleton<pool::ThreadPool>::instance().append(ttk);
		}

	}
}
