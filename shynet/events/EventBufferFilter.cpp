#include "shynet/events/EventBufferFilter.h"
#include "shynet/utils/Logger.h"

namespace shynet
{
	namespace events
	{
		static bufferevent_filter_result inputfiltercb(
			struct evbuffer* source,
			struct evbuffer* destination,
			ev_ssize_t dst_limit,
			enum bufferevent_flush_mode mode,
			void* ctx)
		{
			EventBufferFilter* filter = static_cast<EventBufferFilter*>(ctx);
			std::shared_ptr<Streambuff> tsource = std::make_shared<Streambuff>(source);
			std::shared_ptr<Streambuff> tdest = std::make_shared<Streambuff>(destination);
			return filter->infilter(tsource, tdest);
		}

		static bufferevent_filter_result outputfiltercb(
			struct evbuffer* source,
			struct evbuffer* destination,
			ev_ssize_t dst_limit,
			enum bufferevent_flush_mode mode,
			void* ctx)
		{
			EventBufferFilter* filter = static_cast<EventBufferFilter*>(ctx);
			std::shared_ptr<Streambuff> tsource = std::make_shared<Streambuff>(source);
			std::shared_ptr<Streambuff> tdest = std::make_shared<Streambuff>(destination);
			return filter->outfilter(tsource, tdest);
		}

		EventBufferFilter::EventBufferFilter(const std::shared_ptr<EventBuffer> source)
		{
			buffer_ = bufferevent_filter_new(source->buffer(), inputfiltercb, outputfiltercb,
				BEV_OPT_THREADSAFE, nullptr, this);
			if (buffer_ == nullptr) {
				THROW_EXCEPTION("call bufferevent_filter_new");
			}
		}
		EventBufferFilter::~EventBufferFilter()
		{
			if (buffer_ != nullptr)
				bufferevent_free(buffer_);
		}
		bufferevent_filter_result EventBufferFilter::infilter(const std::shared_ptr<Streambuff> source, const std::shared_ptr<Streambuff> dest)
		{
			return BEV_OK;
		}
		bufferevent_filter_result EventBufferFilter::outfilter(const std::shared_ptr<Streambuff> source, const std::shared_ptr<Streambuff> dest)
		{
			return BEV_OK;
		}
	}
}
