#include "shynet/events/EventBuffer.h"
#include "shynet/utils/Logger.h"

namespace shynet {
	namespace events {
		EventBuffer::EventBuffer(std::shared_ptr<EventBase> base, evutil_socket_t fd, int options) {
			base_ = base;
			buffer_ = bufferevent_socket_new(base->base(), fd, options);
			if (buffer_ == nullptr) {
				throw SHYNETEXCEPTION("call bufferevent_socket_new");
			}
			delflag_ = true;
		}
		EventBuffer::EventBuffer(bufferevent* buffer, bool del) {
			buffer_ = buffer;
			delflag_ = del;
		}
		EventBuffer::EventBuffer(std::shared_ptr<EventBase> base) {
			base_ = base;
			delflag_ = false;
		}
		EventBuffer::~EventBuffer() {
			if (delflag_ == true && buffer_ != nullptr) {
				bufferevent_free(buffer_);
			}
		}
		std::shared_ptr<EventBase> EventBuffer::base() const {
			return base_;
		}
		bufferevent* EventBuffer::buffer() const {
			return buffer_;
		}
		void EventBuffer::set_buffer(bufferevent* buffer) {
			buffer_ = buffer;
			delflag_ = false;
		}
		int EventBuffer::fd() const {
			return bufferevent_getfd(buffer_);
		}
		void EventBuffer::enabled(short what) const {
			bufferevent_enable(buffer_, what);
		}
		void EventBuffer::disable(short what) const {
			bufferevent_disable(buffer_, what);
		}
		short EventBuffer::what() const {
			return bufferevent_get_enabled(buffer_);
		}
		size_t EventBuffer::read(void* data, size_t size) const {
			return bufferevent_read(buffer_, data, size);
		}
		int EventBuffer::read(const std::shared_ptr<Streambuff> buffer) const {
			return bufferevent_read_buffer(buffer_, buffer->buffer());
		}
		int EventBuffer::write(const void* data, size_t size) const {
			return bufferevent_write(buffer_, data, size);
		}
		int EventBuffer::write(const std::shared_ptr<Streambuff> buffer) const {
			return bufferevent_write_buffer(buffer_, buffer->buffer());
		}
		std::shared_ptr<Streambuff> EventBuffer::inputbuffer() const {
			return std::shared_ptr<Streambuff>(new Streambuff(bufferevent_get_input(buffer_)));
		}
		std::shared_ptr<Streambuff> EventBuffer::outputbuffer() const {
			return std::shared_ptr<Streambuff>(new Streambuff(bufferevent_get_output(buffer_)));
		}
		void EventBuffer::setcb(bufferevent_data_cb readcb, bufferevent_data_cb writecb, bufferevent_event_cb eventcb, void* cbarg) const {
			bufferevent_setcb(buffer_, readcb, writecb, eventcb, cbarg);
		}
	}
}
