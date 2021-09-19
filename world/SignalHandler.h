#ifndef WORLD_SIGINTHANDLER_H
#define WORLD_SIGINTHANDLER_H

#include "shynet/events/EventHandler.h"

namespace world {
	/// <summary>
	/// 系统信号处理
	/// </summary>
	class SigIntHandler : public events::EventHandler {
	public:
		explicit SigIntHandler(std::shared_ptr<events::EventBase> base);
		~SigIntHandler();
		void signal(int signal) override;
	};
}

#endif
