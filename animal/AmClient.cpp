#include "animal/AmClient.h"
#include <cstring>
#include <fstream>
#include <json/json.h>
#include <random>
#include <sw/redis++/redis++.h>
namespace redis = sw::redis;
#include <json/json.h>
#include "shynet/pool/MysqlPool.h"
#include "shynet/Logger.h"
#include "shynet/Utility.h"
#include "shynet/IniConfig.h"
#include "shynet/crypto/Url.h"
#include "frmpub/IdWorker.h"
#include "frmpub/protocc/animal.pb.h"
#include "animal/AmClientMgr.h"

namespace animal {
	AmClient::AmClient(std::shared_ptr<net::IPAddress> remote_addr,
		std::shared_ptr<net::IPAddress> listen_addr,
		std::shared_ptr<events::EventBuffer> iobuf)
		: frmpub::Client(remote_addr, listen_addr, iobuf, true, 60, shynet::protocol::FilterProces::ProtoType::WEBSOCKET) {
		GOBJ++;
		LOG_TRACE << "AmClient:" << GOBJ;
		LOG_INFO << "新客户端连接 [ip:" << remote_addr->ip() << ":" << remote_addr->port() << "]";

		pmb_ = {
			{
				protocc::LOGIN_CA_C,
				std::bind(&AmClient::login_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::ENABLED_CA_C,
				std::bind(&AmClient::enabled_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::SETWALLET_CA_C,
				std::bind(&AmClient::setwallet_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::MYTEAM_CA_C,
				std::bind(&AmClient::myteam_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::DEPTH_LIST_CA_C,
				std::bind(&AmClient::depth_list_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::IDCARD_CA_C,
				std::bind(&AmClient::idcard_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::SIGN_CA_C,
				std::bind(&AmClient::sign_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::LOGIN_PWD_CA_C,
				std::bind(&AmClient::login_pwd_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::DISPATCH_COIN_LOG_CA_C,
				std::bind(&AmClient::dispatch_coin_log_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::RECHARGE_CA_C,
				std::bind(&AmClient::recharge_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::TRANSFER_CA_C,
				std::bind(&AmClient::transfer_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::ACTIVATED_COIN_LOG_CA_C,
				std::bind(&AmClient::activated_coin_log_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::SELF_ASSETS_CA_C,
				std::bind(&AmClient::self_assets_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::INCOME_LOG_CA_C,
				std::bind(&AmClient::income_log_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::GETCONTRACT_CA_C,
				std::bind(&AmClient::getcontract_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::GET_COIN_CA_C,
				std::bind(&AmClient::get_coin_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::MAIL_LIST_CA_C,
				std::bind(&AmClient::mail_list_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::MAIL_LOOK_CA_C,
				std::bind(&AmClient::mail_look_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::SMS_CA_C,
				std::bind(&AmClient::sms_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::UPDATA_PWD_CA_C,
				std::bind(&AmClient::updata_pwd_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::OFFICIAL_INFO_CA_C,
				std::bind(&AmClient::official_info_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::REGISTER_CA_C,
				std::bind(&AmClient::register_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::BOOKING_LIST_CA_C,
				std::bind(&AmClient::booking_list_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::CONTRACT_ROB_CA_C,
				std::bind(&AmClient::contract_rob_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::BOOKING_CA_C,
				std::bind(&AmClient::booking_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::ROB_CA_C,
				std::bind(&AmClient::rob_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::ORDER_LIST_CA_C,
				std::bind(&AmClient::order_list_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::SELL_INFO_CA_C,
				std::bind(&AmClient::sell_info_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::PAY_ORDER_CA_C,
				std::bind(&AmClient::pay_order_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::BUY_INFO_CA_C,
				std::bind(&AmClient::buy_info_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::ORDER_OPER_CA_C,
				std::bind(&AmClient::order_oper_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::UPLOAD_CA_C,
				std::bind(&AmClient::upload_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::USER_INFO_CA_C,
				std::bind(&AmClient::user_info_ca_c,this,std::placeholders::_1,std::placeholders::_2)
			},

			//后台推送消息
			{
				protocc::POST_RECHARGE_CS,
				std::bind(&AmClient::post_recharge_cs,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::POST_VERIFIE_CS,
				std::bind(&AmClient::post_verifie_cs,this,std::placeholders::_1,std::placeholders::_2)
			},
		};
	}

	AmClient::~AmClient() {
		GOBJ--;
		LOG_TRACE << "~AmClient:" << GOBJ;
		std::string type = frmpub::Basic::connectname(sif_.st());
		std::string key = shynet::Utility::str_format("%s_%d", type.c_str(), sif_.sid());
		redis::Redis& redis = shynet::Singleton<redis::Redis>::get_instance();
		try {
			redis.del(key);
		}
		catch (const redis::Error& err) {
			LOG_WARN << err.what();
		}
		std::string str;
		if (active()) {
			str = "服务器animal主动关闭连接";
		}
		else {
			str = frmpub::Basic::connectname(sif_.st()) + "客户端主动关闭连接";
		}
		LOG_INFO << str << "[ip:" << remote_addr()->ip() << ":" << remote_addr()->port() << "]";
	}

	int AmClient::input_handle(std::shared_ptr<protocc::CommonObject> obj, std::stack<Envelope>& enves) {
		if (obj != nullptr) {
			if (phone_ == 0) {
				if (obj->msgid() != protocc::POST_VERIFIE_CS &&
					obj->msgid() != protocc::POST_RECHARGE_CS &&
					obj->msgid() != protocc::SMS_CA_C &&
					obj->msgid() != protocc::UPDATA_PWD_CA_C &&
					obj->msgid() != protocc::REGISTER_CA_C &&
					obj->msgid() != protocc::LOGIN_CA_C) {
					LOG_WARN << "未登录" << frmpub::Basic::client_msgname(obj->msgid()) << " 不予处理";
					return -1;
				}
			}
			auto it = pmb_.find(obj->msgid());
			if (it != pmb_.end()) {
				LOG_DEBUG << "input_handle:" << frmpub::Basic::client_msgname(obj->msgid()) << " phone:" << phone_;
				return it->second(obj, enves);
			}
			else {
				LOG_WARN << "消息" << frmpub::Basic::client_msgname(obj->msgid()) << " 没有处理函数";
			}
		}
		return 0;
	}

	void AmClient::close(bool active) {
		frmpub::Client::close(active);
		shynet::Singleton<AmClientMgr>::instance().remove(iobuf()->fd());
	}

	protocc::ServerInfo AmClient::sif() const {
		return sif_;
	}

	/*
	* 排单币或激活币充值成功
	*/
	int AmClient::post_recharge_cs(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::post_recharge_cs msgcs;
		if (msgcs.ParseFromString(data->msgdata()) == true) {
			auto target_client = shynet::Singleton<AmClientMgr>::instance().find(msgcs.phone());
			if (target_client != nullptr) {
				target_client->send_proto(protocc::POST_RECHARGE_CS, &msgcs);
				LOG_DEBUG << "通知post_recharge_cs,phone:" << msgcs.phone();
			}
			return 0;
		}
		else {
			LOG_WARN << "消息" << frmpub::Basic::client_msgname(data->msgid()) << "解析错误";
		}
		return 0;
	}

	/*
	*实名或支付认证成功
	*/
	int AmClient::post_verifie_cs(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::post_verifie_cs msgcs;
		if (msgcs.ParseFromString(data->msgdata()) == true) {
			auto target_client = shynet::Singleton<AmClientMgr>::instance().find(msgcs.phone());
			if (target_client != nullptr) {
				if (msgcs.type() == 1) {
					target_client->name_verifie(3);
				}
				else {
					target_client->pay_verifie(3);
				}
				target_client->send_proto(protocc::POST_VERIFIE_CS, &msgcs);
				LOG_DEBUG << "通知post_verifie_cs,phone:" << msgcs.phone();
			}
			return 0;
		}
		else {
			LOG_WARN << "消息" << frmpub::Basic::client_msgname(data->msgid()) << "解析错误";
		}
		return 0;
	}

	/*
	* 随机邀请码
	*/
	std::string AmClient::rand_invite() {
		std::string new_invite;
		std::string temp1 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		int temp2[] = { 0,1,2,3,4,5,6,7,8,9 };
		int intnum = 0;
		for (size_t i = 0; i < 8; i++) {
			std::default_random_engine random(time(NULL));
			std::uniform_int_distribution<int> dis(0, 1);
			int r1 = dis(random);
			if (intnum < 2) {
				r1 = 0;
			}
			if (r1 == 0) {
				std::uniform_int_distribution<int> dis1(0, int(temp1.size() - 1));
				int r2 = dis(random);
				new_invite += temp1[r2];
			}
			else {
				std::uniform_int_distribution<int> dis1(0, int(sizeof(temp2) / sizeof(temp2[0])) - 1);
				int r2 = dis(random);
				new_invite += std::to_string(temp2[r2]);
				intnum++;
			}
		}
		return new_invite;
	}

	/*
	* 登录认证
	*/
	int AmClient::login_ca_c(std::shared_ptr<protocc::CommonObject> data,
		std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::login_ca_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			protocc::login_ca_s msgs;
			try {
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("call verifi_login(%llu,'%s')",
					msgc.phone(), msgc.pwd().c_str());
				mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
				mysqlx::Row row = sr.fetchOne();
				int result = row.get(0);
				msgs.set_result(result);
				if (result == 0) {
					//顶号处理
					auto target_client = shynet::Singleton<AmClientMgr>::instance().find(msgc.phone());
					if (target_client != nullptr) {
						target_client->send_proto(protocc::POST_OTHER_LOGIN_CS);
						LOG_DEBUG << "用户:" << msgc.phone() << " 已在其他设备登录:" << target_client->remote_addr()->port();
						target_client->close(true);
					}

					phone_ = msgc.phone();
					msgs.set_activated_coin(row.get(1));
					msgs.set_dispatch_coin(row.get(2));
					msgs.set_static_income(row.get(3));
					static_income_ = msgs.static_income();
					msgs.set_promote_income(row.get(4));
					promote_income_ = msgs.promote_income();
					msgs.set_team_income(row.get(5));
					team_income_ = msgs.team_income();
					msgs.set_enable(row.get(6).operator bool());
					activated_ = msgs.enable();
					msgs.set_assets(row.get(7));
					assets_ = msgs.assets();
					msgs.set_level(row.get(8));
					level_ = msgs.level();
					msgs.set_name_verifie(row.get(9));
					name_verifie_ = msgs.name_verifie();
					msgs.set_pay_verifie(row.get(10));
					pay_verifie_ = msgs.pay_verifie();
					msgs.set_nichen(row.get(11).operator std::string());
					nichen_ = msgs.nichen();
					msgs.set_nichen(row.get(11).operator std::string());
					nichen_ = msgs.nichen();
					msgs.set_bank_name(row.get(12).operator std::string());
					bank_name_ = msgs.bank_name();
					msgs.set_bank_cardid(row.get(13).operator std::string());
					bank_cardid_ = msgs.bank_cardid();
					msgs.set_bank_user_name(row.get(14).operator std::string());
					bank_user_name_ = msgs.bank_user_name();
					msgs.set_ailpay_id(row.get(15).operator std::string());
					ailpay_id_ = msgs.ailpay_id();
					msgs.set_alipay_name(row.get(16).operator std::string());
					alipay_name_ = msgs.alipay_name();
					msgs.set_webchat_name(row.get(17).operator std::string());
					webchat_name_ = msgs.webchat_name();
					msgs.set_name(row.get(18).operator std::string());
					name_ = msgs.name();
					msgs.set_idcard(row.get(19).operator std::string());
					id_card_ = msgs.idcard();
					msgs.set_promote_current_income(row.get(20));
					promote_current_income_ = msgs.promote_current_income();
					msgs.set_team_current_income(row.get(21));
					team_current_income_ = msgs.team_current_income();
					msgs.set_sign_time(row.get(22).isNull() ? 0 : row.get(22).operator int());
					trade_ = row.get(23);
					msgs.set_invite(row.get(24).operator std::string());
					invite = msgs.invite();
				}
				LOG_DEBUG << "login_ca_c result:" << result << " activated:" << activated_;
				send_proto(protocc::LOGIN_CA_S, &msgs);
				if (result > 0 && result != 1) {
					return -1;
				}
			}
			catch (const mysqlx::Error& err) {
				LOG_WARN << err.what();
				return 0;
			}
		}
		else {
			LOG_WARN << "消息" << frmpub::Basic::client_msgname(data->msgid()) << "解析错误";
		}
		return 0;
	}

	/*
	* 激活
	*/
	int AmClient::enabled_ca_c(std::shared_ptr<protocc::CommonObject> data,
		std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::enabled_ca_s msgs;
		auto cfg_activeated_coin = atoi(shynet::Singleton<AmClientMgr>::instance().config_data[1].c_str());
		if (activated_) {
			msgs.set_result(2);
		}
		else {
			activated_ = true;
			try {
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("update `user` set activated_coin = activated_coin - %d,dispatch_coin = dispatch_coin + %d,\
activated=1,activatedtime=now() where phone=%llu",
cfg_activeated_coin, cfg_activeated_coin, phone_);
				mysql.fetch()->sql(sql).execute();
				sql = shynet::Utility::str_format("insert into activated_coin_log (phone,time,type,num) VALUES (%llu,now(),3,-%d)",
					phone_, cfg_activeated_coin);
				mysql.fetch()->sql(sql).execute();
				sql = shynet::Utility::str_format("insert into dispatch_coin_log(phone, time, type, num) VALUES(% llu, now(), 8, %d)",
					phone_, cfg_activeated_coin);
				mysql.fetch()->sql(sql).execute();
			}
			catch (const mysqlx::Error& err) {
				LOG_WARN << err.what();
				return 0;
			}
		}
		send_proto(protocc::ENABLED_CA_S, &msgs);
		return 0;
	}

	/*
	* 认证钱包
	*/
	int AmClient::setwallet_ca_c(std::shared_ptr<protocc::CommonObject> data,
		std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::setwallet_ca_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			protocc::setwallet_ca_s msgs;
			try {
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("update `user` set bank_name = '%s',\
bank_cardid='%s',\
bank_user_name='%s',\
ailpay_id='%s',\
alipay_name='%s',\
webchat_name='%s',\
pay_verifie = 2, \
pay_verifie_back = 0 \
where phone=%llu",
msgc.bank_name().c_str(),
msgc.bank_cardid().c_str(),
msgc.bank_user_name().c_str(),
msgc.ailpay_id().c_str(),
msgc.alipay_name().c_str(),
msgc.webchat_name().c_str(),
phone_);
				mysql.fetch()->sql(sql).execute();
				pay_verifie_ = 2;
				msgs.set_result(0);
				msgs.set_pay_verifie(pay_verifie_);
				send_proto(protocc::SETWALLET_CA_S, &msgs);
			}
			catch (const mysqlx::Error& err) {
				LOG_WARN << err.what();
				return 0;
			}
		}
		else {
			LOG_WARN << "消息" << frmpub::Basic::client_msgname(data->msgid()) << "解析错误";
		}
		return 0;
	}

	/*
	* 我的团队
	*/
	int AmClient::myteam_ca_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		try {
			protocc::myteam_ca_s msgs;

			pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
			std::string sql = shynet::Utility::str_format("call myteam_total_assets(%llu)", phone_);
			mysqlx::Row row = mysql.fetch()->sql(sql).execute().fetchOne();
			msgs.set_team_total_assets(row.get(0));
			//LOG_DEBUG << "AAA";
			sql = shynet::Utility::str_format("call depth_user_num(%llu,99)", phone_);
			row = mysql.fetch()->sql(sql).execute().fetchOne();
			msgs.set_team_total_activated_num(row.get(0));
			msgs.set_team_total_register_num(row.get(1));
			//LOG_DEBUG << "BBB";
			sql = shynet::Utility::str_format("call depth_user_num(%llu,1)", phone_);
			row = mysql.fetch()->sql(sql).execute().fetchOne();
			msgs.set_depth1_total_activated_num(row.get(0));
			msgs.set_depth1_total_register_num(row.get(1));
			//LOG_DEBUG << "CCC";
			sql = shynet::Utility::str_format("call depth_user_num(%llu,2)", phone_);
			row = mysql.fetch()->sql(sql).execute().fetchOne();
			msgs.set_depth2_total_activated_num(row.get(0).operator int() - msgs.depth1_total_activated_num());
			msgs.set_depth2_total_register_num(row.get(1).operator int() - msgs.depth1_total_register_num());
			sql = shynet::Utility::str_format("call depth_user_num(%llu,3)", phone_);
			row = mysql.fetch()->sql(sql).execute().fetchOne();
			msgs.set_depth3_total_activated_num(row.get(0).operator int() - msgs.depth1_total_activated_num() - msgs.depth2_total_activated_num());
			msgs.set_depth3_total_register_num(row.get(1).operator int() - msgs.depth1_total_register_num() - msgs.depth2_total_register_num());
			//LOG_DEBUG << row.get(1).operator int() << " AAAA";
			//LOG_DEBUG << msgs.depth1_total_register_num() << "," << msgs.depth2_total_register_num() << "," << msgs.depth3_total_register_num();
			send_proto(protocc::MYTEAM_CA_S, &msgs);
		}
		catch (const mysqlx::Error& err) {
			LOG_WARN << err.what();
			return 0;
		}
		return 0;
	}

	/*
	* 直属玩家
	*/
	int AmClient::depth_list_ca_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		try {
			protocc::depth_list_ca_s msgs;
			pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
			std::string sql = shynet::Utility::str_format("select phone FROM `user` where parent_phone = %llu", phone_);
			mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
			for (auto item : sr.fetchAll()) {
				sql = shynet::Utility::str_format("call assets_num(%llu)", item.get(0).operator std::size_t());
				mysqlx::Row row = mysql.fetch()->sql(sql).execute().fetchOne();
				protocc::depth_list_ca_s_item* it = msgs.add_sifs();
				it->set_nc(row.get(0).operator std::string());
				it->set_num(row.get(1));
				it->set_assets(row.get(2));
			}
			send_proto(protocc::DEPTH_LIST_CA_S, &msgs);
		}
		catch (const mysqlx::Error& err) {
			LOG_WARN << err.what();
			return 0;
		}
		return 0;
	}

	/*
	* 实名认证
	*/
	int AmClient::idcard_ca_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::idcard_ca_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			protocc::idcard_ca_s msgs;

			try {
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("update `user` set name = '%s',\
id_card='%s',\
name_verifie = 2,name_verifie_back = 0 \
where phone=%llu",
msgc.name().c_str(),
msgc.idcard().c_str(),
phone_);
				mysql.fetch()->sql(sql).execute();
				name_verifie_ = 2;
				msgs.set_result(0);
				msgs.set_name_verifie(name_verifie_);
				send_proto(protocc::IDCARD_CA_S, &msgs);
			}
			catch (const mysqlx::Error& err) {
				LOG_WARN << err.what();
				return 0;
			}
		}
		else {
			LOG_WARN << "消息" << frmpub::Basic::client_msgname(data->msgid()) << "解析错误";
		}
		return 0;
	}

	/*
	* 签到
	*/
	int AmClient::sign_ca_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		try {
			protocc::sign_ca_s msgs;
			pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
			std::string sql = shynet::Utility::str_format("call signin(%llu)", phone_);
			mysqlx::Row row = mysql.fetch()->sql(sql).execute().fetchOne();
			msgs.set_result(row.get(0));
			if (msgs.result() == 0) {
				msgs.set_dispatch_coin(row.get(1));
			}
			send_proto(protocc::SIGN_CA_S, &msgs);
		}
		catch (const mysqlx::Error& err) {
			LOG_WARN << err.what();
			return 0;
		}
		return 0;
	}

	/*
	* 修改登录密码
	*/
	int AmClient::login_pwd_ca_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::login_pwd_ca_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			try {
				protocc::login_pwd_ca_s msgs;
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("update `user` set pwd = '%s' where pwd='%s' and phone=%llu",
					msgc.target_pwd().c_str(),
					msgc.source_pwd().c_str(),
					phone_);
				mysqlx::SqlResult rt = mysql.fetch()->sql(sql).execute();
				if (rt.getAffectedItemsCount() == 1) {
					msgs.set_result(0);
				}
				else {
					msgs.set_result(1);
				}
				send_proto(protocc::LOGIN_PWD_CA_S, &msgs);
			}
			catch (const mysqlx::Error& err) {
				LOG_WARN << err.what();
				return 0;
			}
		}
		else {
			LOG_WARN << "消息" << frmpub::Basic::client_msgname(data->msgid()) << "解析错误";
		}
		return 0;
	}

	/*
	* 排单币日志
	*/
	int AmClient::dispatch_coin_log_ca_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		try {
			protocc::dispatch_coin_log_ca_s msgs;
			pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
			std::string sql = shynet::Utility::str_format("select id,UNIX_TIMESTAMP(time),type,target_phone,num,contract_type from dispatch_coin_log\
 where phone=%llu order by time desc limit 20",
				phone_);
			mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
			for (auto item : sr.fetchAll()) {
				protocc::dispatch_coin_log_ca_s_item* it = msgs.add_its();
				it->set_id(item.get(0));
				it->set_time(item.get(1));
				it->set_type(item.get(2));
				it->set_target_phone(item.get(3));
				it->set_num(item.get(4));
				it->set_contract_type(item.get(5));
			}
			send_proto(protocc::DISPATCH_COIN_LOG_CA_S, &msgs);
		}
		catch (const mysqlx::Error& err) {
			LOG_WARN << err.what();
			return 0;
		}

		return 0;
	}

	/*
	* 排单币和激活币充值
	*/
	int AmClient::recharge_ca_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {

		protocc::recharge_ca_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			try {
				protocc::recharge_ca_s msgs;
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("insert into `recharge` (type,num,rmb,time,phone,paycert) values \
(%d,%d,%f,now(),%llu,'%s')",
msgc.type(),
msgc.num(),
msgc.rmb(),
phone_,
msgc.paycert().c_str());
				mysqlx::SqlResult rt = mysql.fetch()->sql(sql).execute();
				if (rt.getAffectedItemsCount() == 1) {
					msgs.set_result(0);
				}
				else {
					msgs.set_result(1);
				}
				send_proto(protocc::RECHARGE_CA_S, &msgs);
			}
			catch (const mysqlx::Error& err) {
				LOG_WARN << err.what();
				return 0;
			}
		}
		else {
			LOG_WARN << "消息" << frmpub::Basic::client_msgname(data->msgid()) << "解析错误";
		}
		return 0;
	}

	/*
	* 排单币和激活币赠送
	*/
	int AmClient::transfer_ca_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::transfer_ca_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			protocc::transfer_ca_s msgs;
			if (msgc.target_phone() == phone_) {
				msgs.set_result(3);
			}
			else {
				try {
					pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
					std::string sql = shynet::Utility::str_format("call transfer(%d,%d,%llu,%llu,%d)",
						msgc.type(),
						msgc.num(),
						phone_,
						msgc.target_phone(),
						msgc.trade_code());
					mysqlx::Row row = mysql.fetch()->sql(sql).execute().fetchOne();
					msgs.set_result(row.get(0));
					if (msgs.result() == 0) {
						msgs.set_type(msgc.type());
						msgs.set_curr_num(row.get(1));

						auto target_client = shynet::Singleton<AmClientMgr>::instance().find(msgc.target_phone());
						if (target_client != nullptr) {
							protocc::transfer_ca_cs msgcs;
							msgcs.set_type(msgc.type());
							msgcs.set_curr_num(row.get(2));
							target_client->send_proto(protocc::TRANSFER_CA_CS, &msgcs);
						}
					}
				}
				catch (const mysqlx::Error& err) {
					LOG_WARN << err.what();
					return 0;
				}
			}
			send_proto(protocc::TRANSFER_CA_S, &msgs);
		}
		else {
			LOG_WARN << "消息" << frmpub::Basic::client_msgname(data->msgid()) << "解析错误";
		}
		return 0;
	}

	/*
	* 派单币日志
	*/
	int AmClient::activated_coin_log_ca_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		try {
			protocc::activated_coin_log_ca_s msgs;
			pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
			std::string sql = shynet::Utility::str_format("select id,UNIX_TIMESTAMP(time),type,target_phone,num from activated_coin_log\
 where phone=%llu order by time desc limit 20",
				phone_);
			mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
			for (auto item : sr.fetchAll()) {
				protocc::activated_coin_log_ca_s_item* it = msgs.add_its();
				it->set_id(item.get(0));
				it->set_time(item.get(1));
				it->set_type(item.get(2));
				it->set_target_phone(item.get(3));
				it->set_num(item.get(4));
			}
			send_proto(protocc::ACTIVATED_COIN_LOG_CA_S, &msgs);
		}
		catch (const mysqlx::Error& err) {
			LOG_WARN << err.what();
			return 0;
		}

		return 0;
	}

	/*
	* 我的资产
	*/
	int AmClient::self_assets_ca_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		try {
			protocc::self_assets_ca_s msgs;
			pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
			std::string sql = shynet::Utility::str_format("SELECT contract_info.contract_type,\
contract_info.current_price,\
UNIX_TIMESTAMP(user_contract.gettime),\
user_contract.getway \
FROM user_contract INNER JOIN contract_info on contract_info.id = user_contract.contract_id WHERE user_contract.phone = %llu",
phone_);
			mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
			for (auto item : sr.fetchAll()) {
				protocc::self_assets_ca_s_item* it = msgs.add_its();
				it->set_type(item.get(0));
				it->set_price(item.get(1));
				it->set_time(item.get(2));
				it->set_getway(item.get(3));
			}
			send_proto(protocc::SELF_ASSETS_CA_S, &msgs);
		}
		catch (const mysqlx::Error& err) {
			LOG_WARN << err.what();
			return 0;
		}
		return 0;
	}

	/*
	* 收益日志
	*/
	int AmClient::income_log_ca_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::income_log_ca_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			try {
				protocc::income_log_ca_s msgs;
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("SELECT contract_info.contract_type,\
income_log.price,\
UNIX_TIMESTAMP(time),\
depth,\
target_phone \
FROM income_log INNER JOIN contract_info on income_log.contract_id = contract_info.id \
where income_log.phone = %llu and income_log.type = %d order by time desc",
phone_,
msgc.type());
				mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
				for (auto item : sr.fetchAll()) {
					protocc::income_log_ca_s_item* it = msgs.add_its();
					it->set_type(item.get(0));
					it->set_price(item.get(1));
					it->set_time(item.get(2));
					it->set_depth(item.get(3).isNull() ? 0 : item.get(3).operator int());
					it->set_target_phone(item.get(4).isNull() ? 0 : item.get(4).operator int64_t());
				}
				send_proto(protocc::INCOME_LOG_CA_S, &msgs);
			}
			catch (const mysqlx::Error& err) {
				LOG_WARN << err.what();
				return 0;
			}
		}
		else {
			LOG_WARN << "消息" << frmpub::Basic::client_msgname(data->msgid()) << "解析错误";
		}
		return 0;
	}

	/*
	* 换购合约
	*/
	int AmClient::getcontract_ca_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {

		protocc::getcontract_ca_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			try {
				protocc::getcontract_ca_s msgs;
				msgs.set_change_type(msgc.change_type());
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("call getcontract(%f,%d,%d,%llu,%d)",
					msgc.price(),
					msgc.contract_type(),
					msgc.change_type(),
					phone_,
					msgc.trade_code());
				mysqlx::Row row = mysql.fetch()->sql(sql).execute().fetchOne();
				msgs.set_result(row.get(0));
				if (msgs.result() == 0) {
					if (msgs.change_type() == 1) {
						promote_current_income_ = (float)(row.get(1).operator double());
						msgs.set_income(promote_current_income_);
					}
					else if (msgs.change_type() == 2) {
						team_current_income_ = (float)(row.get(1).operator double());
						msgs.set_income(team_current_income_);
					}
				}
				send_proto(protocc::GETCONTRACT_CA_S, &msgs);
			}
			catch (const mysqlx::Error& err) {
				LOG_WARN << err.what();
				return 0;
			}
		}
		else {
			LOG_WARN << "消息" << frmpub::Basic::client_msgname(data->msgid()) << "解析错误";
		}
		return 0;
	}

	/*
	* 换购排单币和激活币
	*/
	int AmClient::get_coin_ca_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::get_coin_ca_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			try {
				protocc::get_coin_ca_s msgs;
				msgs.set_change_type(msgc.change_type());
				msgs.set_coin_type(msgc.coin_type());
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("call getcoin(%d,%d,%d,%llu,%d)",
					msgc.num(),
					msgc.change_type(),
					msgc.coin_type(),
					phone_,
					msgc.trade_code());
				mysqlx::Row row = mysql.fetch()->sql(sql).execute().fetchOne();
				msgs.set_result(row.get(0));
				if (msgs.result() == 0) {
					if (msgs.change_type() == 1) {
						promote_current_income_ = float(row.get(1).operator double());
						msgs.set_income(promote_current_income_);
					}
					else if (msgs.change_type() == 2) {
						team_current_income_ = float(row.get(1).operator double());
						msgs.set_income(team_current_income_);
					}
					if (msgs.coin_type() == 1) {
						msgs.set_coin_value(int(row.get(2).operator double()));
					}
					else if (msgs.coin_type() == 2) {
						msgs.set_coin_value(int(row.get(2).operator double()));
					}
				}
				send_proto(protocc::GET_COIN_CA_S, &msgs);
			}
			catch (const mysqlx::Error& err) {
				LOG_WARN << err.what();
				return 0;
			}
		}
		else {
			LOG_WARN << "消息" << frmpub::Basic::client_msgname(data->msgid()) << "解析错误";
		}
		return 0;
	}

	/*
	* 邮件列表
	*/
	int AmClient::mail_list_ca_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		try {
			protocc::mail_list_ca_s msgs;
			pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
			std::string sql = shynet::Utility::str_format("SELECT id,topic,info,UNIX_TIMESTAMP(time),\
(SELECT count(notice_id) FROM look_notice WHERE notice_id = notice.id and look_notice.phone = %llu)\
 FROM notice order by time desc limit 20",
				phone_);
			mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
			for (auto item : sr.fetchAll()) {
				protocc::mail_list_ca_s_item* it = msgs.add_its();
				it->set_id(item.get(0));
				it->set_topic(item.get(1).operator std::string());
				it->set_info(item.get(2).operator std::string());
				it->set_time(item.get(3));
				it->set_state(item.get(4));
			}
			send_proto(protocc::MAIL_LIST_CA_S, &msgs);
		}
		catch (const mysqlx::Error& err) {
			LOG_WARN << err.what();
			return 0;
		}
		return 0;
	}

	/*
	* 查看邮件
	*/
	int AmClient::mail_look_ca_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::mail_look_ca_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			try {
				protocc::mail_look_ca_s msgs;
				msgs.set_id(msgc.id());
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("insert look_notice (notice_id,phone) values (%d,%llu)",
					msgs.id(),
					phone_);
				mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
				if (sr.getAffectedItemsCount() == 1) {
					msgs.set_result(0);
					msgs.set_state(1);
				}
				else {
					msgs.set_result(1);
				}
				send_proto(protocc::MAIL_LOOK_CA_S, &msgs);
			}
			catch (const mysqlx::Error& err) {
				LOG_WARN << err.what();
				return 0;
			}
		}
		else {
			LOG_WARN << "消息" << frmpub::Basic::client_msgname(data->msgid()) << "解析错误";
		}
		return 0;
	}

	/*
	* 发送短信验证码
	*/
	int AmClient::sms_ca_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::sms_ca_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {

			std::default_random_engine random(time(NULL));
			std::uniform_int_distribution<int> dis(100001, 999999);
			int sms_code = dis(random);
			shynet::IniConfig& ini = shynet::Singleton<shynet::IniConfig>::instance(std::move(std::string("animal.ini").c_str()));
			std::string url = ini.get<const char*, std::string>("sms", "url", "http://utf8.api.smschinese.cn/");
			std::string uid = ini.get<const char*, std::string>("sms", "Uid", "mingsheng1985");
			std::string key = ini.get<const char*, std::string>("sms", "Key", "d41d8cd98f00b204e980");
			std::string phone = (phone_ == 0 ? std::to_string(msgc.phone()) : std::to_string(phone_));
			std::string text = ini.get<const char*, std::string>("sms", "smsText", "验证码：%d 。该验证码仅用于注册，请勿泄露给他人使用！");
			text = crypto::url::escapeURL(shynet::Utility::str_format(text, sms_code));
			std::string order = shynet::Utility::str_format("%s?Uid=%s&Key=%s&smsMob=%s&smsText=%s", url.c_str(), uid.c_str(), key.c_str(), phone.c_str(), text.c_str());
			std::string result = frmpub::Sms::send(order);
			int ret = atoi(result.c_str());
			LOG_DEBUG << phone << " 发送短信验证码结果:" << ret;
			protocc::sms_ca_s msgs;
			msgs.set_result(ret > 0 ? 0 : 1);
			if (msgs.result() == 0) {
				try {
					msgs.set_code(sms_code);
					redis::Redis& redis = shynet::Singleton<redis::Redis>::get_instance();
					std::string codekey = "code_" + phone + "_" + std::to_string(sms_code);
					redis.set(codekey, std::to_string(sms_code), std::chrono::seconds(60 * 3));
				}
				catch (const redis::Error& err) {
					LOG_WARN << err.what();
					return 0;
				}

			}
			send_proto(protocc::SMS_CA_S, &msgs);
		}
		else {
			LOG_WARN << "消息" << frmpub::Basic::client_msgname(data->msgid()) << "解析错误";
		}
		return 0;
	}

	/*
	* 更新交易或登录密码
	*/
	int AmClient::updata_pwd_ca_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::updata_pwd_ca_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			protocc::updata_pwd_ca_s msgs;
			msgs.set_type(msgc.type());
			int64_t phone = phone_ == 0 ? msgc.phone() : phone_;
			std::string codekey = "code_" + std::to_string(phone) + "_" + std::to_string(msgc.code());
			try {
				redis::Redis& redis = shynet::Singleton<redis::Redis>::get_instance();
				if (redis.get(codekey).operator bool() == false) {
					msgs.set_result(2);
				}
				else {
					if (redis.get(codekey).value() != std::to_string(msgc.code())) {
						msgs.set_result(1);
					}
					else {
						try {
							pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
							std::string sql;
							if (msgs.type() == 1) {
								sql = shynet::Utility::str_format("update `user` set trade = %d where phone = %llu", atoi(msgc.newpwd().c_str()), phone);
							}
							else {
								sql = shynet::Utility::str_format("update `user` set pwd = '%s' where phone = %llu", msgc.newpwd().c_str(), phone);
							}
							mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
							if (sr.getAffectedItemsCount() == 1) {
								msgs.set_result(0);
								if (msgs.type() == 1) {
									trade_ = atoi(msgc.newpwd().c_str());
								}
							}
							else {
								msgs.set_result(0);
							}
						}
						catch (const mysqlx::Error& err) {
							LOG_WARN << err.what();
							return 0;
						}
					}
				}
			}
			catch (const mysqlx::Error& err) {
				LOG_WARN << err.what();
				return 0;
			}
			send_proto(protocc::UPDATA_PWD_CA_S, &msgs);
		}
		else {
			LOG_WARN << "消息" << frmpub::Basic::client_msgname(data->msgid()) << "解析错误";
		}
		return 0;
	}

	/*
	* 官方充值信息
	*/
	int AmClient::official_info_ca_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {

		try {
			protocc::official_info_ca_s msgs;
			pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
			std::string sql = shynet::Utility::str_format("call official_info()");
			mysqlx::Row row = mysql.fetch()->sql(sql).execute().fetchOne();
			msgs.set_ailpay_name(row.get(0).operator std::string());
			msgs.set_ailpay_account(row.get(1).operator std::string());
			msgs.set_ail_qr(row.get(2).operator std::string());
			msgs.set_bank_card(row.get(3).operator std::string());
			msgs.set_bank_name(row.get(4).operator std::string());
			msgs.set_bank_account(row.get(5).operator std::string());
			send_proto(protocc::OFFICIAL_INFO_CA_S, &msgs);
		}
		catch (const mysqlx::Error& err) {
			LOG_WARN << err.what();
			return 0;
		}
		return 0;
	}

	/*
	* 注册
	*/
	int AmClient::register_ca_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {

		protocc::register_ca_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			protocc::register_ca_s msgs;
			std::string codekey = "code_" + std::to_string(msgc.phone()) + "_" + std::to_string(msgc.code());
			try {
				redis::Redis& redis = shynet::Singleton<redis::Redis>::get_instance();
				if (redis.get(codekey).operator bool() == false) {
					msgs.set_result(4);
				}
				else {
					if (redis.get(codekey).value() != std::to_string(msgc.code())) {
						msgs.set_result(1);
					}
					try {
						pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
						std::string sql = shynet::Utility::str_format("call register(%llu,'%s','%s',%d,'%s')",
							msgc.phone(),
							msgc.nichen().c_str(),
							msgc.pwd().c_str(),
							msgc.trade(),
							msgc.invite().c_str());
						mysqlx::Row row = mysql.fetch()->sql(sql).execute().fetchOne();
						msgs.set_result(row.get(0));
					}
					catch (const mysqlx::Error& err) {
						LOG_WARN << err.what();
						return 0;
					}
				}
			}
			catch (const redis::Error& err) {
				LOG_WARN << err.what();
				return 0;
			}
			send_proto(protocc::REGISTER_CA_S, &msgs);
		}
		else {
			LOG_WARN << "消息" << frmpub::Basic::client_msgname(data->msgid()) << "解析错误";
		}
		return 0;
	}

	/*
	* 预约列表
	*/
	int AmClient::booking_list_ca_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		try {
			protocc::booking_list_ca_s msgs;
			pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
			std::string sql = shynet::Utility::str_format("SELECT id,contract_type,UNIX_TIMESTAMP(time) \
FROM booking where phone = %llu order by time desc limit 20",
phone_);
			mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
			for (auto item : sr.fetchAll()) {
				protocc::booking_list_ca_s_item* it = msgs.add_its();
				it->set_id(item.get(0));
				it->set_contract_type(item.get(1));
				it->set_time(item.get(2));
			}
			send_proto(protocc::BOOKING_LIST_CA_S, &msgs);
		}
		catch (const mysqlx::Error& err) {
			LOG_WARN << err.what();
			return 0;
		}
		return 0;
	}

	/*
	* 合约抢购区
	*/
	int AmClient::contract_rob_ca_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		try {
			protocc::contract_rob_ca_s msgs;
			pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
			std::string sql = shynet::Utility::str_format("call contract_rob(%llu)", phone_);
			mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
			for (auto item : sr.fetchAll()) {
				protocc::contract_rob_ca_s_item* it = msgs.add_its();
				it->set_contract_type(item.get(0));
				it->set_rob(item.get(1));
			}
			send_proto(protocc::CONTRACT_ROB_CA_S, &msgs);
		}
		catch (const mysqlx::Error& err) {
			LOG_WARN << err.what();
			return 0;
		}
		return 0;
	}

	/*
	* 预约
	*/
	int AmClient::booking_ca_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::booking_ca_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			try {
				protocc::booking_ca_s msgs;
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("call booking(%llu,%d)", phone_, msgc.contract_type());
				mysqlx::Row row = mysql.fetch()->sql(sql).execute().fetchOne();
				msgs.set_result(row.get(0));
				if (msgs.result() == 0) {
					msgs.set_curr_dispatch_coin(row.get(1));
				}
				send_proto(protocc::BOOKING_CA_S, &msgs);
			}
			catch (const mysqlx::Error& err) {
				LOG_WARN << err.what();
				return 0;
			}
		}
		else {
			LOG_WARN << "消息" << frmpub::Basic::client_msgname(data->msgid()) << "解析错误";
		}
		return 0;
	}

	/*
	* 抢购
	*/
	int AmClient::rob_ca_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::rob_ca_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			try {
				redis::Redis& redis = shynet::Singleton<redis::Redis>::get_instance();
				protocc::rob_ca_s msgs;
				std::string robkey = "user_isrob_" + std::to_string(phone_) + "_" + std::to_string(msgc.contract_type());
				redis::OptionalString isrob = redis.get(robkey);
				if (isrob && *isrob == "true") {
					msgs.set_result(2);
				}
				else {
					int sec;//过期时间
					mysqlx::Row row;
					pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
					{
						std::string sql = shynet::Utility::str_format("call rob(%llu,%d)", phone_, msgc.contract_type());
						row = mysql.fetch()->sql(sql).execute().fetchOne();
						msgs.set_result(row.get(0));
					}
					if (msgs.result() == 0) {
						sec = row.get(1);
						redis.set(robkey, "true", std::chrono::seconds(sec));
						std::string key = "user_rob_contract_type_" + std::to_string(msgc.contract_type());
						std::string iscreatekey = "create_contract_type_" + std::to_string(msgc.contract_type());
						if (redis.exists(key) == 0 && redis.exists(iscreatekey) == 0) {
							std::string sql = shynet::Utility::str_format("call ripe_contract(%d)", msgc.contract_type());
							mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
							std::unordered_set<std::string> contracts;
							for (auto item : sr.fetchAll()) {
								Json::Value root;
								root["contract_id"] = item.get(0).operator int();
								root["sell_phone"] = item.get(1).operator int64_t();
								Json::FastWriter fw;
								contracts.insert(fw.write(root));
							}
							if (contracts.empty() == false) {
								redis.sadd(key, contracts.begin(), contracts.end());
								redis.expire(key, sec);
								LOG_DEBUG << "生成合约";
							}
							redis.set(iscreatekey, "true", std::chrono::seconds(sec));
						}
						redis::OptionalString target_contract = redis.srandmember(key);
						if (!target_contract) {
							msgs.set_result(7);
							//返还预约排单币
							std::string sql = shynet::Utility::str_format("call rob_result(0,%llu,%d,0,0,0)",
								phone_, msgc.contract_type());
							row = mysql.fetch()->sql(sql).execute().fetchOne();
							msgs.set_curr_dispatch_coin(row.get(0));
							LOG_DEBUG << "没有成熟的合约,合约抢购失败,返还预约排单币";
						}
						else {
							Json::Reader rd;
							Json::Value target_contract_josn;
							rd.parse(target_contract.value(), target_contract_josn);

							int64_t orderid = shynet::Singleton<frmpub::IdWorker>::instance(1, 1).getid();
							std::string sql = shynet::Utility::str_format("call rob_result(1,%llu,%d,%llu,%d,%llu)",
								phone_,
								msgc.contract_type(),
								orderid,
								target_contract_josn["contract_id"].asInt(),
								target_contract_josn["sell_phone"].asInt64(),
								phone_
							);
							if (target_contract_josn["sell_phone"].asInt64() == phone_) {
								msgs.set_result(7);
								//返还预约排单币
								std::string sql = shynet::Utility::str_format("call rob_result(0,%llu,%d,0,0,0)",
									phone_, msgc.contract_type());
								row = mysql.fetch()->sql(sql).execute().fetchOne();
								msgs.set_curr_dispatch_coin(row.get(0));
								LOG_DEBUG << "不能抢购自己的活跃,合约抢购失败,返还预约排单币";
							}
							else {
								redis.srem(key, target_contract.value());
								row = mysql.fetch()->sql(sql).execute().fetchOne();
								msgs.set_curr_dispatch_coin(row.get(0));
								LOG_DEBUG << "获得合约,phone:" << phone_ << " orderid:" << orderid;
							}
						}
					}
				}
				LOG_DEBUG << "抢购结果:" << msgs.result();
				send_proto(protocc::ROB_CA_S, &msgs);
			}
			catch (const mysqlx::Error& err) {
				LOG_WARN << err.what();
				return 0;
			}
			catch (const redis::Error& err) {
				LOG_WARN << err.what();
				return 0;
			}
		}
		else {
			LOG_WARN << "消息" << frmpub::Basic::client_msgname(data->msgid()) << "解析错误";
		}
		return 0;
	}

	/*
	* 抢单记录或转让记录
	*/
	int AmClient::order_list_ca_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::order_list_ca_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			try {
				protocc::order_list_ca_s msgs;
				msgs.set_type(msgc.type());

				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string field = "sell_phone";
				if (msgc.type() == 1) {
					field = "buy_phone";
				}
				std::string sql = shynet::Utility::str_format("SELECT `order`.id,\
contract_type.id,\
UNIX_TIMESTAMP(`order`.rush_time),\
UNIX_TIMESTAMP(`order`.pay_time),\
UNIX_TIMESTAMP(`order`.sell_confirm_time),\
`order`.state, \
`order`.sell_phone,\
`order`.buy_phone,\
`order`.getway,\
`order`.contract_price,\
contract_type.interest_rate \
FROM `order` INNER JOIN contract_info on contract_id = contract_info.id \
INNER JOIN contract_type on contract_info.contract_type = contract_type.id \
 where %s = %llu order by `order`.id desc limit 100",
					field.c_str(),
					phone_);
				mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
				for (auto item : sr.fetchAll()) {
					protocc::order_list_ca_s_item* it = msgs.add_its();
					it->set_id(item.get(0));
					it->set_contract_type(item.get(1));
					it->set_robtime(item.get(2).isNull() ? 0 : item.get(2).operator int());
					it->set_paytime(item.get(3).isNull() ? 0 : item.get(3).operator int());
					it->set_oktime(item.get(4).isNull() ? 0 : item.get(4).operator int());
					it->set_state(item.get(5).isNull() ? 0 : item.get(5).operator int());
					it->set_sell_phone(item.get(6));
					it->set_buy_phone(item.get(7));
					double current_price = item.get(9);
					it->set_price((float)current_price);
				}
				send_proto(protocc::ORDER_LIST_CA_S, &msgs);
			}
			catch (const mysqlx::Error& err) {
				LOG_WARN << err.what();
				return 0;
			}
		}
		else {
			LOG_WARN << "消息" << frmpub::Basic::client_msgname(data->msgid()) << "解析错误";
		}
		return 0;
	}

	/*
	* 卖家信息
	*/
	int AmClient::sell_info_ca_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::sell_info_ca_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			try {
				protocc::sell_info_ca_s msgs;

				std::string sql = shynet::Utility::str_format("select nichen,bank_name,bank_cardid,bank_user_name,ailpay_id,alipay_name,webchat_name,phone \
from `user` where phone = %llu",
msgc.sell_phone());
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				mysqlx::Row row = mysql.fetch()->sql(sql).execute().fetchOne();
				msgs.set_nichen(row.get(0).operator std::string());
				msgs.set_bank_name(row.get(1).operator std::string());
				msgs.set_bank_cardid(row.get(2).operator std::string());
				msgs.set_bank_user_name(row.get(3).operator std::string());
				msgs.set_ailpay_id(row.get(4).operator std::string());
				msgs.set_alipay_name(row.get(5).operator std::string());
				msgs.set_webchat_name(row.get(6).operator std::string());
				msgs.set_sell_phone(row.get(7));

				send_proto(protocc::SELL_INFO_CA_S, &msgs);
			}
			catch (const mysqlx::Error& err) {
				LOG_WARN << err.what();
				return 0;
			}
		}
		else {
			LOG_WARN << "消息" << frmpub::Basic::client_msgname(data->msgid()) << "解析错误";
		}
		return 0;
	}

	/*
	* 支付订单
	*/
	int AmClient::pay_order_ca_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::pay_order_ca_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			try {
				protocc::pay_order_ca_s msgs;

				std::string sql = shynet::Utility::str_format("UPDATE `order` set payicon='%s',state=2,pay_time=now() where id = %lld", msgc.pay_voucher().c_str(), msgc.order_id());
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				mysqlx::SqlResult st = mysql.fetch()->sql(sql).execute();
				if (st.getAffectedItemsCount() > 0) {
					msgs.set_result(0);
				}
				else {
					msgs.set_result(1);
				}

				send_proto(protocc::PAY_ORDER_CA_S, &msgs);
			}
			catch (const mysqlx::Error& err) {
				LOG_WARN << err.what();
				return 0;
			}
		}
		else {
			LOG_WARN << "消息" << frmpub::Basic::client_msgname(data->msgid()) << "解析错误";
		}
		return 0;
	}

	/*
	* 买家信息
	*/
	int AmClient::buy_info_ca_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::buy_info_ca_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			try {
				protocc::buy_info_ca_s msgs;

				std::string sql = shynet::Utility::str_format("SELECT nichen,phone,payicon FROM `order` \
INNER JOIN `user` ON order.buy_phone = `user`.phone \
WHERE `order`.id = %llu",
msgc.order_id());
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				mysqlx::Row row = mysql.fetch()->sql(sql).execute().fetchOne();
				msgs.set_nichen(row.get(0).operator std::string());
				msgs.set_phone(row.get(1));
				msgs.set_pay_voucher(row.get(2).operator std::string());

				send_proto(protocc::BUY_INFO_CA_S, &msgs);
			}
			catch (const mysqlx::Error& err) {
				LOG_WARN << err.what();
				return 0;
			}
		}
		else {
			LOG_WARN << "消息" << frmpub::Basic::client_msgname(data->msgid()) << "解析错误";
		}
		return 0;
	}

	/*
	* 卖家订单操作
	*/
	int AmClient::order_oper_ca_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::order_oper_ca_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			try {
				protocc::order_oper_ca_s msgs;
				msgs.set_oper_type(msgc.oper_type());
				if (msgc.oper_type() == 2) {
					std::string sql = shynet::Utility::str_format("UPDATE `order` set state = 4 where id=%llu", msgc.order_id());
					pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
					mysqlx::SqlResult st = mysql.fetch()->sql(sql).execute();
					if (st.getAffectedItemsCount() > 0) {
						msgs.set_result(0);
					}
					else {
						msgs.set_result(1);
					}
				}
				else {
					if (trade_ != msgc.trade_code()) {
						msgs.set_result(2);
					}
					else {
						std::string sql = shynet::Utility::str_format("call confirm_order(%llu)", msgc.order_id());
						pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
						mysqlx::Row row = mysql.fetch()->sql(sql).execute().fetchOne();
						msgs.set_result(row.get(0));
					}
				}
				send_proto(protocc::ORDER_OPER_CA_S, &msgs);
			}
			catch (const mysqlx::Error& err) {
				LOG_WARN << err.what();
				return 0;
			}
		}
		else {
			LOG_WARN << "消息" << frmpub::Basic::client_msgname(data->msgid()) << "解析错误";
		}
		return 0;
	}

	int AmClient::upload_ca_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::upload_ca_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			shynet::IniConfig& ini = shynet::Singleton<shynet::IniConfig>::get_instance();
			std::string uploadpath = ini.get<const char*, std::string>("animal", "uploadpath", "/usr/share/nginx/html/upload/");

			std::ofstream out;
			out.open(uploadpath + msgc.name(), std::ios::binary | std::ios::trunc);
			out.write(msgc.data().c_str(), msgc.data().length());
			out.close();
			LOG_DEBUG << "保存文件:" << uploadpath << msgc.name();
			protocc::upload_ca_s msgs;
			send_proto(protocc::UPLOAD_CA_S, &msgs);
		}
		else {
			LOG_WARN << "消息" << frmpub::Basic::client_msgname(data->msgid()) << "解析错误";
		}
		return 0;
	}

	int AmClient::user_info_ca_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::user_info_ca_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			try {
				protocc::user_info_ca_s msgs;
				std::string sql = shynet::Utility::str_format("SELECT phone,\
activated_coin,\
dispatch_coin,\
static_income,\
promote_income,\
team_income,\
promote_current_income,\
team_current_income,\
(SELECT SUM(contract_info.current_price) FROM user_contract INNER JOIN contract_info ON contract_info.id = user_contract.contract_id where phone = %llu)\
 FROM `user` WHERE phone = %llu;",
					msgc.phone(),
					msgc.phone());
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				mysqlx::Row row = mysql.fetch()->sql(sql).execute().fetchOne();
				msgs.set_phone(row.get(0));
				msgs.set_activated_coin(row.get(1));
				msgs.set_dispatch_coin(row.get(2));
				msgs.set_static_income(row.get(3));
				msgs.set_promote_income(row.get(4));
				msgs.set_team_income(row.get(5));
				msgs.set_promote_current_income(row.get(6));
				msgs.set_team_current_income(row.get(7));
				msgs.set_assets(row.get(8).isNull() ? 0.f : (float)(row.get(8).operator double()));
				if (msgs.phone() == phone_) {
					static_income_ = msgs.static_income();
					promote_income_ = msgs.promote_income();
					team_income_ = msgs.team_income();
					assets_ = msgs.assets();
				}
				send_proto(protocc::USER_INFO_CA_S, &msgs);
			}
			catch (const mysqlx::Error& err) {
				LOG_WARN << err.what();
				return 0;
			}
		}
		else {
			LOG_WARN << "消息" << frmpub::Basic::client_msgname(data->msgid()) << "解析错误";
		}
		return 0;
	}


}
