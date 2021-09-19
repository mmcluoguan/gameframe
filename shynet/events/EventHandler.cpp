#include "shynet/events/EventHandler.h"
#include "shynet/utils/Logger.h"

namespace shynet
{
	namespace events
	{

		static void cbfunc(evutil_socket_t fd, short what, void* arg)
		{
			EventHandler* handler = static_cast<EventHandler*>(arg);
			if (what & EV_TIMEOUT)
			{
				handler->timeout(fd);
			}
			else if (what & EV_READ)
			{
				handler->input(fd);
			}
			else if (what & EV_WRITE)
			{
				handler->output(fd);
			}
			else if (what & EV_SIGNAL)
			{
				handler->signal(fd);
			}
			else
			{
				LOG_WARN << "what=" << what;
			}
		}

		EventHandler::EventHandler(std::shared_ptr<EventBase> base, evutil_socket_t fd, short what)
		{
			base_ = base;
			event_ = event_new(base->base(), fd, what, cbfunc, this);
			if (event_ == nullptr)
			{
				THROW_EXCEPTION("call event_new");
			}
		}
		EventHandler::EventHandler()
		{
		}

		EventHandler::~EventHandler()
		{
			if (event_ != nullptr)
			{
				event_del(event_);
				event_free(event_);
			}
			event_ = nullptr;
		}

		void EventHandler::event(std::shared_ptr<EventBase> base, evutil_socket_t fd, short what)
		{
			if (event_ != nullptr) {
				THROW_EXCEPTION("event 已经存在");
			}
			base_ = base;
			event_ = event_new(base->base(), fd, what, cbfunc, this);
			if (event_ == nullptr) {
				THROW_EXCEPTION("call event_new");
			}
		}

		struct event* EventHandler::event() const
		{
			return event_;
		}

		evutil_socket_t EventHandler::fd() const
		{
			return event_get_fd(event_);
		}

		std::shared_ptr<EventBase> EventHandler::base() const
		{
			return base_;
		}

		short EventHandler::what() const
		{
			return event_get_events(event_);
		}
	}
}
