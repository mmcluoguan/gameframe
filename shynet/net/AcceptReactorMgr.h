#pragma once
#include <list>
#include "shynet/thread/AcceptThread.h"
#include <shynet/Singleton.h>

namespace shynet {
	namespace net {

		class AcceptReactorMgr final : public Nocopy {
			friend class Singleton<AcceptReactorMgr>;
			AcceptReactorMgr();
		public:
			~AcceptReactorMgr();

			void notify(const void* data, size_t len);
		private:
			std::shared_ptr<thread::AcceptThread> find_rtk() const;
		};
	}
}
