#pragma once
#include "shynet/net/ListenEvent.h"
#include "shynet/pool/ThreadPool.h"
#include <unordered_map>

namespace shynet {
	namespace net {
		class ListenReactorMgr final : public Nocopy {
			friend class utils::Singleton<ListenReactorMgr>;
			ListenReactorMgr();
		public:
			~ListenReactorMgr();

			int add(std::shared_ptr<ListenEvent> v);
			bool remove(int k);
			std::shared_ptr<ListenEvent> find(int k);
		private:
			void notify(const void* data, size_t len);
		private:
			std::mutex les_mutex_;
			/*
			* 监听列表
			*/
			std::unordered_map<int, std::shared_ptr<ListenEvent>> les_;
		};
	}
}