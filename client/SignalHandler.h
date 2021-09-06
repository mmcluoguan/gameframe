#pragma once
#include "shynet/events/EventHandler.h"

namespace client
{
	class SigIntHandler : public events::EventHandler
	{
	public:
		explicit SigIntHandler(std::shared_ptr<events::EventBase> base);
		~SigIntHandler();
		void signal(int signal) override;
	};
}
