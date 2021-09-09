#pragma once
#include "shynet/events/EventHandler.h"

namespace animal {
	class StdinHandler : public events::EventHandler {
	public:
		StdinHandler(std::shared_ptr<events::EventBase> base, evutil_socket_t fd);
		~StdinHandler();
		void input(int fd) override;
	private:
		void quit_order(const char* od, int argc, char** argv, const char* optarg);
		void redis_order(const char* od, int argc, char** argv, const char* optarg);
	};
}
