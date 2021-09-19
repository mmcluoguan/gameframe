#pragma once
#include "shynet/events/EventBuffer.h"
#include "shynet/thread/Thread.h"

namespace shynet {
	namespace thread {

		class ListenThread : public Thread {
		public:
			explicit ListenThread(size_t index);
			~ListenThread();

			int run() override;
			int stop() override;
			int notify(const void* data, size_t len) const;
			void process(struct bufferevent* bev);
		private:
			std::shared_ptr<events::EventBuffer> pair_[2] = { 0 };
			std::shared_ptr<events::EventBase> base_ = nullptr;
		};
	}
}