#pragma once
#include <sw/redis++/redis++.h>
namespace redis = sw::redis;
#include <unordered_map>
#include <functional>
#include "shynet/events/EventBuffer.h"
#include "frmpub/Client.h"

namespace backend {
	class BkClient : public frmpub::Client {
	public:
		BkClient(std::shared_ptr<net::IPAddress> remote_addr,
			std::shared_ptr<net::IPAddress> listen_addr,
			std::shared_ptr<events::EventBuffer> iobuf);
		~BkClient();

		int input_handle(std::shared_ptr<protocc::CommonObject> obj, std::stack<Envelope>& enves) override;
		void close(bool active) override;

		protocc::ServerInfo sif() const;
		int64_t phone() const {
			return phone_;
		}
	private:
		int login_bk_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int crucial_bk_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int maillist_bk_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int addmail_bk_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int contract_type_list_bk_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int update_contract_type_bk_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int add_contract_bk_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int booking_count_bk_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int booking_bk_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int ripe_count_bk_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int ripe_bk_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int order_list_bk_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int order_bk_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int recharge_bk_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int recharge_oper_bk_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int user_list_bk_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int update_user_bk_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int user_contract_list_bk_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int del_user_contract_bk_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int name_verifie_list_bk_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int name_verifie_oper_bk_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int pay_verifie_list_bk_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int pay_verifie_oper_bk_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int create_user_bk_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int user_proxy_bk_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int close_user_bk_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int new_recharge_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int contract_oper_list_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int contract_oper_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
	private:
		int64_t phone_;
	};
}
