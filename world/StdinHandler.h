#pragma once
#include "shynet/events/EventHandler.h"

namespace world {
	/// <summary>
	/// 终端输入处理
	/// </summary>
	class StdinHandler : public events::EventHandler {
	public:
		StdinHandler(std::shared_ptr<events::EventBase> base, evutil_socket_t fd);
		~StdinHandler();
		void input(int fd) override;
	private:
		void quit_order(const char* od, int argc, char** argv, const char* optarg);
	};
}
