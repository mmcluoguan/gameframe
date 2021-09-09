#pragma once
#include "frmpub/Connector.h"

namespace backend {
	class AmConnector : public frmpub::Connector {
	public:
		AmConnector(std::shared_ptr<net::IPAddress> connect_addr);
		~AmConnector();

		void complete() override;
		int input_handle(std::shared_ptr<protocc::CommonObject> obj, std::stack<Envelope>& enves) override;

		static int am_id;
		static std::shared_ptr<AmConnector> am_connector();
	private:
		int register_gate_dbvisit_s(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
	private:
	};
}
