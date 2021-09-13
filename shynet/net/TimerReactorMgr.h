#pragma once
#include <mutex>
#include <unordered_map>
#include "shynet/net/TimerEvent.h"
#include "shynet/utils/Singleton.h"

namespace shynet {
	namespace net {
		class TimerReactorMgr final : public Nocopy {
			friend class utils::Singleton<TimerReactorMgr>;
			TimerReactorMgr();
		public:
			~TimerReactorMgr();

			int add(std::shared_ptr<TimerEvent> v);
			bool remove(int k);
			std::shared_ptr<TimerEvent> find(int k);
		private:
			void notify(const void* data, size_t len);
		private:
			std::mutex times_mutex_;
			std::unordered_map<int, std::shared_ptr<TimerEvent>> times_;
		};
	}
}
