#include "shynet/net/ConnectIoBuffer.h"
#include "shynet/pool/ThreadPool.h"
#include "shynet/net/TimerReactorMgr.h"
#include "shynet/task/ConnectReadIoTask.h"

namespace shynet {
	namespace net {
		static void ioreadcb(struct bufferevent* bev, void* ptr) {
			ConnectIoBuffer* p = reinterpret_cast<ConnectIoBuffer*>(ptr);
			p->io_readcb();
		}

		static void iowritecb(struct bufferevent* bev, void* ptr) {
			ConnectIoBuffer* p = reinterpret_cast<ConnectIoBuffer*>(ptr);
			p->io_writecb();
		}

		static void ioeventcb(struct bufferevent* bev, short events, void* ptr) {
			ConnectIoBuffer* p = reinterpret_cast<ConnectIoBuffer*>(ptr);
			p->io_eventcb(events);
		}

		ConnectIoBuffer::ConnectIoBuffer(std::shared_ptr<events::EventBase> base, bool enable_ssl, SSL_CTX* ctx) :
			events::EventBuffer(base) {
			if (enable_ssl) {
				iobuf_ = std::shared_ptr<events::EventBufferSsl>(new events::EventBufferSsl(base, -1,
					BUFFEREVENT_SSL_CONNECTING,
					BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE, ctx));
			}
			else {
				iobuf_ = std::shared_ptr<events::EventBuffer>(new events::EventBuffer(base, -1,
					BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE));
			}
			buffer(iobuf_->buffer());

			iobuf_->setcb(ioreadcb, iowritecb, ioeventcb, this);
			iobuf_->enabled(EV_READ | EV_WRITE | EV_PERSIST);
		}
		ConnectIoBuffer::~ConnectIoBuffer() {
		}

		std::weak_ptr<ConnectEvent> ConnectIoBuffer::cnev() const {
			return cnev_;
		}

		void ConnectIoBuffer::cnev(std::weak_ptr<ConnectEvent> cnev) {
			cnev_ = cnev;
		}


		void ConnectIoBuffer::io_readcb() {
			std::shared_ptr<ConnectEvent> shconector = cnev_.lock();
			if (shconector != nullptr) {
				std::shared_ptr<task::ConnectReadIoTask> io(new task::ConnectReadIoTask(shconector));
				utils::Singleton<pool::ThreadPool>::instance().appendWork(io, fd());
			}
		}

		void ConnectIoBuffer::io_writecb() {
			std::shared_ptr<ConnectEvent> shconector = cnev_.lock();
			if (shconector != nullptr) {
				int ret = shconector->output();
				if (ret == -1) {
					shconector->close(ConnectEvent::CloseType::CLIENT_CLOSE);
				}
			}
		}

		void ConnectIoBuffer::io_eventcb(short events) {
			std::shared_ptr<ConnectEvent> shconector = cnev_.lock();
			if (shconector != nullptr) {
				if (events & BEV_EVENT_CONNECTED) {
					if (shconector->enable_heart()) {
						std::shared_ptr<ConnectHeartbeat> ht(
							new ConnectHeartbeat(shconector, { shconector->heart_second() ,0L }));
						utils::Singleton<TimerReactorMgr>::instance().add(ht);
						shconector->heart(ht);
					}
					shconector->success();
				}
				else if (events & BEV_EVENT_EOF) {
					shconector->close(ConnectEvent::CloseType::SERVER_CLOSE);
				}
				else if (events & (BEV_EVENT_ERROR | BEV_EVENT_READING | BEV_EVENT_WRITING)) {
					if (shconector->dnsbase() != nullptr)
						LOG_WARN << evutil_gai_strerror(bufferevent_socket_get_dns_error(iobuf_->buffer()));
					LOG_WARN << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR());
					shconector->close(ConnectEvent::CloseType::CONNECT_FAIL);
				}
			}
		}

	}
}
