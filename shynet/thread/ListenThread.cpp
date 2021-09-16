#include "shynet/thread/ListenThread.h"
#include "shynet/net/ListenReactorMgr.h"
#include "shynet/utils/Logger.h"

namespace shynet {

	extern pthread_barrier_t g_barrier;

	namespace thread {
		ListenThread::ListenThread(size_t index) : Thread(ThreadType::LISTEN, index) {
		}

		ListenThread::~ListenThread() {
		}

		int ListenThread::notify(const void* data, size_t len) const {
			return pair_[0]->write(data, len);
		}

		static void pipeReadcb(struct bufferevent* bev, void* ptr) {
			ListenThread* rtk = reinterpret_cast<ListenThread*>(ptr);
			rtk->process(bev);
		}

		void ListenThread::process(bufferevent* bev) {
			events::EventBuffer pbuf(bev);
			char buf[sizeof(int)] = { 0 };
			do {
				size_t len = pbuf.read(&buf, sizeof(buf));
				if (len == 0) {
					break;
				}
				else if (len != sizeof(buf)) {
					LOG_WARN << "没有足够的数据";
				}
				else {
					int serverid = 0;
					size_t index = 0;
					memcpy(&serverid, buf + index, sizeof(serverid));
					index += sizeof(serverid);

					std::shared_ptr<net::ListenEvent> listenEv =
						utils::Singleton<net::ListenReactorMgr>::instance().find(serverid);
					if (listenEv != nullptr) {
						listenEv->event(base_, listenEv->listenfd(), EV_READ | EV_PERSIST);
						base_->addevent(listenEv, nullptr);
					}
				}
			} while (true);
		}


		int ListenThread::run() {
			try {
				LOG_TRACE << "ListenThread::run threadtype:" << (int)type();

				base_ = std::shared_ptr<events::EventBase>(new events::EventBase());
				base_->make_pair_buffer(pair_);
				pair_[0]->enabled(EV_WRITE);
				pair_[0]->disable(EV_READ);
				pair_[1]->enabled(EV_READ);
				pair_[1]->disable(EV_WRITE);
				pair_[1]->setcb(pipeReadcb, nullptr, nullptr, this);
				pthread_barrier_wait(&g_barrier);
				base_->loop(EVLOOP_NO_EXIT_ON_EMPTY);
				pair_[0].reset();
				pair_[1].reset();
			}
			catch (const std::exception& err) {
				LOG_WARN << err.what();
			}
			return 0;
		}
		int ListenThread::stop() {
			base_->loopexit();
			return 0;
		}
	}
}
