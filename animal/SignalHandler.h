﻿#pragma once
#include "shynet/events/EventHandler.h"

namespace animal {
	class SigIntHandler : public events::EventHandler {
	public:
		explicit SigIntHandler(std::shared_ptr<events::EventBase> base);
		~SigIntHandler();
		void signal(int signal) override;
	};
}
