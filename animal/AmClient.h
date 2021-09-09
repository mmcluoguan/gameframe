#pragma once
#include <sw/redis++/redis++.h>
namespace redis = sw::redis;
#include <unordered_map>
#include <functional>
#include "shynet/events/EventBuffer.h"
#include "frmpub/Client.h"

namespace animal {
	class AmClient : public frmpub::Client {
	public:
		AmClient(std::shared_ptr<net::IPAddress> remote_addr,
			std::shared_ptr<net::IPAddress> listen_addr,
			std::shared_ptr<events::EventBuffer> iobuf);
		~AmClient();

		int input_handle(std::shared_ptr<protocc::CommonObject> obj, std::stack<Envelope>& enves) override;
		void close(bool active) override;

		protocc::ServerInfo sif() const;
		int64_t phone() const {
			return phone_;
		}
		void name_verifie(int v) {
			name_verifie_ = v;
		}
		void pay_verifie(int v) {
			pay_verifie_ = v;
		}
	private:
		int login_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int enabled_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int setwallet_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int myteam_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int depth_list_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int idcard_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int sign_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int login_pwd_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int dispatch_coin_log_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int recharge_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int transfer_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int activated_coin_log_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int self_assets_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int income_log_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int getcontract_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int get_coin_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int mail_list_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int mail_look_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int sms_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int updata_pwd_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int official_info_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int register_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int booking_list_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int contract_rob_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int booking_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int rob_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int order_list_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int sell_info_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int pay_order_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int buy_info_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int order_oper_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int upload_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int user_info_ca_c(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);

		//后台推送消息
		int post_recharge_cs(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
		int post_verifie_cs(std::shared_ptr<protocc::CommonObject> data,
			std::stack<protocol::FilterProces::Envelope>& enves);
	private:
		static std::string rand_invite();
	private:
		int64_t phone_ = 0;
		float static_income_ = 0.;
		float promote_income_ = 0.;
		float team_income_ = 0.;
		bool activated_ = false;
		float assets_ = 0.f;
		int level_ = 0;
		int name_verifie_ = 1;
		int pay_verifie_ = 1;
		std::string nichen_;
		std::string bank_name_;
		std::string bank_cardid_;
		std::string bank_user_name_;
		std::string ailpay_id_;
		std::string alipay_name_;
		std::string webchat_name_;
		std::string name_;
		std::string id_card_;
		float promote_current_income_ = 0.;
		float team_current_income_ = 0.;
		std::string invite;
		int trade_ = 0;
	};
}
