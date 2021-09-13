#include "shynet/thread/ConnectThread.h"
#include <cstring>
#include <event2/dns.h>
#include "shynet/net/ConnectReactorMgr.h"
#include "shynet/net/ConnectIoBuffer.h"
#include "shynet/utils/Logger.h"

namespace shynet {

	extern pthread_barrier_t g_barrier;

	namespace thread {
		ConnectThread::ConnectThread(size_t index) : Thread(ThreadType::CONNECT, index) {
		}

		ConnectThread::~ConnectThread() {
		}

		static void pipeReadcb(struct bufferevent* bev, void* ptr) {
			ConnectThread* cnt = reinterpret_cast<ConnectThread*>(ptr);
			cnt->process(bev);
		}


		void ConnectThread::process(bufferevent* bev) {
			events::EventBuffer pbuf(bev);
			char buf[sizeof(int)] = { 0 };
			do {
				size_t len = pbuf.read(&buf, sizeof(buf));
				if (len == 0) {
					break;
				}
				else if (len != sizeof(buf)) {
					LOG_ERROR << "not enough data";
				}
				else {
					int connectid = 0;
					size_t index = 0;
					memcpy(&connectid, buf + index, sizeof(connectid));
					index += sizeof(connectid);

					std::shared_ptr<net::ConnectEvent> connect =
						utils::Singleton<net::ConnectReactorMgr>::instance().find(connectid);
					if (connect != nullptr) {
						struct sockaddr* address = (struct sockaddr*)connect->connect_addr()->sockaddr();

						std::shared_ptr<net::ConnectIoBuffer> buffer;
						if (connect->enable_ssl()) {
							buffer = std::shared_ptr<net::ConnectIoBuffer>(
								new net::ConnectIoBuffer(base_, true, connect->ctx()));

						}
						else {
							buffer = std::shared_ptr<net::ConnectIoBuffer>(
								new net::ConnectIoBuffer(base_, false));
						}
						buffer->set_cnev(connect);
						connect->iobuf(buffer);
						if (connect->enable_dns()) {
							evdns_base* dnsbase = evdns_base_new(base_->base(), 1);
							if (dnsbase == nullptr) {
								LOG_ERROR << "call connect_hostname";
							}
							connect->dnsbase(dnsbase);
							bufferevent_socket_connect_hostname(buffer->buffer(), dnsbase, AF_UNSPEC,
								connect->hostname().c_str(), connect->dnsport());
						}
						else {
							if (bufferevent_socket_connect(buffer->buffer(), address, sizeof(sockaddr_storage)) == -1) {
								LOG_ERROR << "call bufferevent_socket_connect";
							}
						}
					}
				}
			} while (true);
		}


		int ConnectThread::run() {
			LOG_TRACE << "ConnectThread::run threadtype:" << (int)type();

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
			return 0;
		}
		int ConnectThread::stop() {
			base_->loopexit();
			return 0;
		}


		int ConnectThread::notify(const void* data, size_t len) const {
			return pair_[0]->write(data, len);
		}
	}
}
