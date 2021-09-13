#pragma once
#include <list>
#include "shynet/thread/AcceptThread.h"
#include <shynet/utils/Singleton.h>

namespace shynet {
	namespace net {

		class AcceptReactorMgr final : public Nocopy {
			friend class utils::Singleton<AcceptReactorMgr>;
			AcceptReactorMgr();
		public:
			~AcceptReactorMgr();

			void notify(const void* data, size_t len);
		private:
			std::shared_ptr<thread::AcceptThread> find_rtk() const;
		};
	}
}
