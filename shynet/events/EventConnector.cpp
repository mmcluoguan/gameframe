#include "shynet/events/EventConnector.h"
#include "shynet/utils/Logger.h"
#include <event2/dns.h>
#include <cstring>

namespace shynet {
	namespace events {
		static void readcb(struct bufferevent* bev, void* ptr) {
			EventConnector* conector = static_cast<EventConnector*>(ptr);
			if (conector->buffer()->buffer() != bev) {
				THROW_EXCEPTION("call readcb");
			}
			conector->input(conector->buffer());
		}

		static void writecb(struct bufferevent* bev, void* ptr) {
			EventConnector* conector = static_cast<EventConnector*>(ptr);
			if (conector->buffer()->buffer() != bev) {
				THROW_EXCEPTION("call writecb");
			}
			conector->output(conector->buffer());
		}

		static void eventcb(struct bufferevent* bev, short events, void* ptr) {
			EventConnector* conector = static_cast<EventConnector*>(ptr);
			if (conector->buffer()->buffer() != bev) {
				THROW_EXCEPTION("call eventcb");
			}
			if (events & BEV_EVENT_CONNECTED) {
				conector->success(conector->buffer());
			}
			else if (events & (BEV_EVENT_ERROR | BEV_EVENT_EOF)) {
				if (conector->dnsbase() != nullptr)
					LOG_WARN << evutil_gai_strerror(bufferevent_socket_get_dns_error(bev));
				LOG_WARN << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR());
				conector->error(conector->buffer());
			}
		}

		EventConnector::EventConnector(std::shared_ptr<EventBase> base) {
			base_ = base;
		}

		EventConnector::~EventConnector() {
			if (dnsbase_ != nullptr)
				evdns_base_free(dnsbase_, 0);
			if (ctx_ != nullptr) {
				SSL_CTX_free(ctx_);
			}
		}

		std::shared_ptr<EventBase> EventConnector::base() const {
			return base_;
		}

		std::shared_ptr<EventBuffer> EventConnector::buffer() const {
			return buffer_;
		}

		evdns_base* EventConnector::dnsbase() const {
			return dnsbase_;
		}

		evutil_socket_t EventConnector::fd() const {
			return bufferevent_getfd(buffer_->buffer());
		}

		int EventConnector::connect_hostname(const char* hostname, int port) {
			dnsbase_ = evdns_base_new(base_->base(), 1);
			if (dnsbase_ == nullptr) {
				THROW_EXCEPTION("call evdns_base_new");
			}
			buffer_ = std::make_shared<EventBuffer>(base_, -1, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);
			buffer_->setcb(readcb, writecb, eventcb, this);
			buffer_->enabled(EV_READ | EV_WRITE);
			return bufferevent_socket_connect_hostname(buffer_->buffer(), dnsbase_, AF_UNSPEC, hostname, port);
		}

		int EventConnector::connect_ssl(sockaddr* address, int addrlen) {
			ctx_ = SSL_CTX_new(SSLv23_client_method());
			if (ctx_ == nullptr) {
				THROW_EXCEPTION("call SSL_CTX_new");
			}
			bufferssl_ = std::make_shared<EventBufferSsl>(base_, -1, BUFFEREVENT_SSL_CONNECTING,
				BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE, ctx_);
			bufferssl_->setcb(readcb, writecb, eventcb, this);
			bufferssl_->enabled(EV_READ | EV_WRITE);
			return bufferevent_socket_connect(bufferssl_->buffer(), address, addrlen);
		}

		int EventConnector::connect_hostname_ssl(const char* hostname, int port) {
			dnsbase_ = evdns_base_new(base_->base(), 1);
			if (dnsbase_ == nullptr) {
				THROW_EXCEPTION("call evdns_base_new");
			}
			ctx_ = SSL_CTX_new(SSLv23_client_method());
			if (ctx_ == nullptr) {
				THROW_EXCEPTION("call SSL_CTX_new");
			}
			bufferssl_ = std::make_shared<EventBufferSsl>(base_, -1, BUFFEREVENT_SSL_CONNECTING,
				BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE, ctx_);
			bufferssl_->setcb(readcb, writecb, eventcb, this);
			bufferssl_->enabled(EV_READ | EV_WRITE);
			return bufferevent_socket_connect_hostname(bufferssl_->buffer(), dnsbase_, AF_UNSPEC, hostname, port);
		}

		int EventConnector::connect(sockaddr* address, int addrlen) {
			buffer_ = std::make_shared<EventBuffer>(base_, -1, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);
			buffer_->setcb(readcb, writecb, eventcb, this);
			buffer_->enabled(EV_READ | EV_WRITE);
			return bufferevent_socket_connect(buffer_->buffer(), address, addrlen);
		}
	}
}
