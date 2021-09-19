#include "shynet/events/Streambuff.h"
#include "shynet/utils/Logger.h"

namespace shynet
{
	namespace events
	{
		Streambuff::Streambuff()
		{
			delflag_ = true;
			buffer_ = evbuffer_new();
			if (buffer_ == nullptr){
				throw SHYNETEXCEPTION("call evbuffer_new");
			}
		}
		Streambuff::Streambuff(evbuffer* buffer)
		{
			delflag_ = false;
			buffer_ = buffer;
		}
		Streambuff::~Streambuff()
		{
			if (delflag_ && buffer_ != nullptr)
				evbuffer_free(buffer_);
		}
		evbuffer* Streambuff::buffer() const
		{
			return buffer_;
		}
		void Streambuff::lock() const
		{
			evbuffer_lock(buffer_);
		}
		void Streambuff::unlock() const
		{
			evbuffer_unlock(buffer_);
		}
		size_t Streambuff::length() const
		{
			return evbuffer_get_length(buffer_);
		}
		int Streambuff::add(const void* data, size_t len) const
		{
			return evbuffer_add(buffer_, data, len);
		}
		int Streambuff::addprintf(const char* fmt, ...) const
		{
			va_list argList;
			va_start(argList, fmt);
			int ret = evbuffer_add_vprintf(buffer_, fmt, argList);
			va_end(argList);
			return ret;
		}
		int Streambuff::prepend(const void* data, size_t len) const
		{
			return evbuffer_prepend(buffer_, data, len);
		}
		int Streambuff::expand(size_t len) const
		{
			return evbuffer_expand(buffer_, len);
		}
		int Streambuff::addbuffer(const std::shared_ptr<Streambuff> src) const
		{
			return evbuffer_add_buffer(buffer_, src->buffer());
		}
		int Streambuff::prependbuffer(const std::shared_ptr<Streambuff> src) const
		{
			return evbuffer_prepend_buffer(buffer_, src->buffer());
		}
		int Streambuff::remove(void* data, size_t len) const
		{
			return evbuffer_remove(buffer_, data, len);
		}
		int Streambuff::removebuffer(const std::shared_ptr<Streambuff> dst, size_t len) const
		{
			return evbuffer_remove_buffer(buffer_, dst->buffer(), len);
		}
		int Streambuff::drain(size_t len) const
		{
			return evbuffer_drain(buffer_, len);
		}
		ssize_t Streambuff::copyout(void* data, size_t len) const
		{
			return evbuffer_copyout(buffer_, data, len);
		}
		char* Streambuff::readln(size_t* n_read_out, evbuffer_eol_style style) const
		{
			return evbuffer_readln(buffer_, n_read_out, style);
		}
		evbuffer_ptr Streambuff::search(const char* what, size_t len, const evbuffer_ptr* start) const
		{
			return evbuffer_search(buffer_, what, len, start);
		}
		evbuffer_ptr Streambuff::search_range(const char* what, size_t len, const evbuffer_ptr* start, const evbuffer_ptr* end) const
		{
			return evbuffer_search_range(buffer_, what, len, start, end);
		}
		evbuffer_ptr Streambuff::search_eol(evbuffer_ptr* start, size_t* eol_len_out, evbuffer_eol_style eol_style) const
		{
			return evbuffer_search_eol(buffer_, start, eol_len_out, eol_style);
		}
		int Streambuff::ptr_set(struct evbuffer_ptr* pos, size_t position, enum evbuffer_ptr_how how) const
		{
			return evbuffer_ptr_set(buffer_, pos, position, how);
		}
		int Streambuff::peek(ssize_t len, evbuffer_ptr* start_at, evbuffer_iovec* vec_out, int n_vec) const
		{
			return evbuffer_peek(buffer_, len, start_at, vec_out, n_vec);
		}
		int Streambuff::reserve_space(ssize_t size, evbuffer_iovec* vec, int n_vecs) const
		{
			return evbuffer_reserve_space(buffer_, size, vec, n_vecs);
		}
		int Streambuff::commit_space(evbuffer_iovec* vec, int n_vecs) const
		{
			return evbuffer_commit_space(buffer_, vec, n_vecs);
		}
		int Streambuff::write(evutil_socket_t fd) const
		{
			return evbuffer_write(buffer_, fd);
		}
		int Streambuff::write_atmost(evutil_socket_t fd, ssize_t howmuch) const
		{
			return evbuffer_write_atmost(buffer_, fd, howmuch);
		}
		int Streambuff::read(evutil_socket_t fd, int howmuch) const
		{
			return evbuffer_read(buffer_, fd, howmuch);
		}
		int Streambuff::add_reference(const void* data, size_t datlen, evbuffer_ref_cleanup_cb cleanupfn, void* extra) const
		{
			return evbuffer_add_reference(buffer_, data, datlen, cleanupfn, extra);
		}
		unsigned char* Streambuff::pullup(ssize_t size) const
		{
			return evbuffer_pullup(buffer_, size);
		}
	}
}
