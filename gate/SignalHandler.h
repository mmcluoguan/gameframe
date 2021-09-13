#pragma once
#include "shynet/events/EventHandler.h"

namespace gate
{
	/// <summary>
	/// 系统信号处理
	/// </summary>
	class SigIntHandler : public events::EventHandler
	{
	public:
		explicit SigIntHandler(std::shared_ptr<events::EventBase> base);
		~SigIntHandler();
		void signal(int signal) override;
	};
}
