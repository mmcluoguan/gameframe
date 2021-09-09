#include "backend/BkClient.h"
#include <cstring>
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
#include "frmpub/protocc/backend.pb.h"
#include "frmpub/protocc/common.pb.h"
#include "backend/BkClientMgr.h"
#include "backend/AmConnector.h"

namespace backend {
	BkClient::BkClient(std::shared_ptr<net::IPAddress> remote_addr,
		std::shared_ptr<net::IPAddress> listen_addr,
		std::shared_ptr<events::EventBuffer> iobuf)
		: frmpub::Client(remote_addr, listen_addr, iobuf, true, 60, shynet::protocol::FilterProces::ProtoType::WEBSOCKET) {
		GOBJ++;
		LOG_TRACE << "BkClient:" << GOBJ;
		LOG_INFO << "新客户端连接 [ip:" << remote_addr->ip() << ":" << remote_addr->port() << "]";

		pmb_ = {
			{
				protocc::LOGIN_BK_C,
				std::bind(&BkClient::login_bk_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::CRUCIAL_BK_C,
				std::bind(&BkClient::crucial_bk_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::MAILLIST_BK_C,
				std::bind(&BkClient::maillist_bk_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::ADDMAIL_BK_C,
				std::bind(&BkClient::addmail_bk_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::CONTRACT_TYPE_LIST_BK_C,
				std::bind(&BkClient::contract_type_list_bk_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::UPDATE_CONTRACT_TYPE_BK_C,
				std::bind(&BkClient::update_contract_type_bk_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::ADD_CONTRACT_BK_C,
				std::bind(&BkClient::add_contract_bk_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::BOOKING_COUNT_BK_C,
				std::bind(&BkClient::booking_count_bk_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::BOOKING_BK_C,
				std::bind(&BkClient::booking_bk_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::RIPE_COUNT_BK_C,
				std::bind(&BkClient::ripe_count_bk_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::RIPE_BK_C,
				std::bind(&BkClient::ripe_bk_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::ORDER_LIST_BK_C,
				std::bind(&BkClient::order_list_bk_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::ORDER_BK_C,
				std::bind(&BkClient::order_bk_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::RECHARGE_BK_C,
				std::bind(&BkClient::recharge_bk_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::RECHARGE_OPER_BK_C,
				std::bind(&BkClient::recharge_oper_bk_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::USER_LIST_BK_C,
				std::bind(&BkClient::user_list_bk_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::UPDATE_USER_BK_C,
				std::bind(&BkClient::update_user_bk_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::USER_CONTRACT_LIST_BK_C,
				std::bind(&BkClient::user_contract_list_bk_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::DEL_USER_CONTRACT_BK_C,
				std::bind(&BkClient::del_user_contract_bk_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::NAME_VERIFIE_LIST_BK_C,
				std::bind(&BkClient::name_verifie_list_bk_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::NAME_VERIFIE_OPER_BK_C,
				std::bind(&BkClient::name_verifie_oper_bk_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::PAY_VERIFIE_LIST_BK_C,
				std::bind(&BkClient::pay_verifie_list_bk_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::PAY_VERIFIE_OPER_BK_C,
				std::bind(&BkClient::pay_verifie_oper_bk_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::CREATE_USER_BK_C,
				std::bind(&BkClient::create_user_bk_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::USER_PROXY_BK_C,
				std::bind(&BkClient::user_proxy_bk_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::CLOSE_USER_BK_C,
				std::bind(&BkClient::close_user_bk_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::NEW_RECHARGE_C,
				std::bind(&BkClient::new_recharge_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::CONTRACT_OPER_LIST_C,
				std::bind(&BkClient::contract_oper_list_c,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::CONTRACT_OPER_C,
				std::bind(&BkClient::contract_oper_c,this,std::placeholders::_1,std::placeholders::_2)
			},
		};
	}

	BkClient::~BkClient() {
		GOBJ--;
		LOG_TRACE << "~BkClient:" << GOBJ;
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
			str = "服务器backend主动关闭连接";
		}
		else {
			str = frmpub::Basic::connectname(sif_.st()) + "客户端主动关闭连接";
		}
		LOG_INFO << str << "[ip:" << remote_addr()->ip() << ":" << remote_addr()->port() << "]";
	}

	int BkClient::input_handle(std::shared_ptr<protocc::CommonObject> obj, std::stack<Envelope>& enves) {
		if (obj != nullptr) {
			if (phone_ == 0) {
				if (obj->msgid() != protocc::LOGIN_BK_C) {
					LOG_WARN << "未登录" << frmpub::Basic::client_msgname(obj->msgid()) << " 不予处理";
					return -1;
				}
			}
			auto it = pmb_.find(obj->msgid());
			if (it != pmb_.end()) {
				if (obj->msgid() != protocc::NEW_RECHARGE_C)
					LOG_DEBUG << "input_handle:" << frmpub::Basic::client_msgname(obj->msgid()) << " phone:" << phone_;
				return it->second(obj, enves);
			}
			else {
				LOG_WARN << "消息" << frmpub::Basic::client_msgname(obj->msgid()) << " 没有处理函数";
			}
		}
		return 0;
	}

	void BkClient::close(bool active) {
		frmpub::Client::close(active);
		shynet::Singleton<BkClientMgr>::instance().remove(iobuf()->fd());
	}

	protocc::ServerInfo BkClient::sif() const {
		return sif_;
	}

	/*
	* 后台登录
	*/
	int BkClient::login_bk_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::login_bk_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			protocc::login_bk_s msgs;
			try {
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("select count(phone) from back_account where phone=%llu and pwd = '%s'",
					msgc.phone(), msgc.pwd().c_str());
				mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
				mysqlx::Row row = sr.fetchOne();
				int result = row.get(0);
				if (result > 0) {
					phone_ = msgc.phone();
					msgs.set_result(0);
				}
				else {
					msgs.set_result(1);
				}
				send_proto(protocc::LOGIN_BK_S, &msgs);
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
	* 关键指标
	*/
	int BkClient::crucial_bk_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::crucial_bk_s msgs;
		try {
			pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
			std::string sql = shynet::Utility::str_format("call back_crucial()");
			mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
			mysqlx::Row row = sr.fetchOne();
			msgs.set_total_register_num(row.get(0));
			msgs.set_today_register_num(row.get(1));
			msgs.set_total_active_num(row.get(2));
			msgs.set_today_active_num(row.get(3));
			msgs.set_total_disenable_num(row.get(4));
			msgs.set_today_disenable_num(row.get(5));
			msgs.set_total_income(row.get(6));

			send_proto(protocc::CRUCIAL_BK_S, &msgs);
		}
		catch (const mysqlx::Error& err) {
			LOG_WARN << err.what();
			return 0;
		}
		return 0;
	}

	/*
	* 公告列表
	*/
	int BkClient::maillist_bk_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		try {
			protocc::maillist_bk_s msgs;
			pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
			std::string sql = shynet::Utility::str_format("SELECT id,topic,info,UNIX_TIMESTAMP(time) FROM notice order by time desc",
				phone_);
			mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
			for (auto item : sr.fetchAll()) {
				protocc::maillist_bk_s_item* it = msgs.add_its();
				it->set_id(item.get(0));
				it->set_topic(item.get(1).operator std::string());
				it->set_info(item.get(2).operator std::string());
				it->set_time(item.get(3));
			}
			send_proto(protocc::MAILLIST_BK_S, &msgs);
		}
		catch (const mysqlx::Error& err) {
			LOG_WARN << err.what();
			return 0;
		}
		return 0;
	}

	/*
	* 添加公告
	*/
	int BkClient::addmail_bk_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::addmail_bk_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			protocc::addmail_bk_s msgs;
			try {
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("insert into notice (topic,info,time) values('%s','%s',now())",
					msgc.topic().c_str(),
					msgc.info().c_str());
				mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
				if (sr.getAffectedItemsCount() > 0) {
					msgs.set_result(0);
				}
				else {
					msgs.set_result(1);
				}
				send_proto(protocc::ADDMAIL_BK_S, &msgs);
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
	* 合约类型列表
	*/
	int BkClient::contract_type_list_bk_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		try {
			protocc::contract_type_list_bk_s msgs;
			pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
			std::string sql = shynet::Utility::str_format("SELECT id,cycle,interest_rate,reserve_dispatch_coin,rush_dispathch_coin,\
rush_begin_time,rush_end_time,state,name,min_price,max_price\
 FROM contract_type");
			mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
			for (auto item : sr.fetchAll()) {
				protocc::contract_type_list_bk_s_item* it = msgs.add_its();
				it->set_id(item.get(0));
				it->set_cycle(item.get(1));
				it->set_interest_rate(item.get(2));
				it->set_reserve_dispatch_coin(item.get(3));
				it->set_rush_dispathch_coin(item.get(4));
				it->set_rush_begin_time(item.get(5));
				it->set_rush_end_time(item.get(6));
				it->set_state(item.get(7));
				it->set_name(item.get(8).operator std::string());
				it->set_min_price(item.get(9));
				it->set_max_price(item.get(10));
			}
			send_proto(protocc::CONTRACT_TYPE_LIST_BK_S, &msgs);
		}
		catch (const mysqlx::Error& err) {
			LOG_WARN << err.what();
			return 0;
		}
		return 0;
	}

	/*
	* 修改合约
	*/
	int BkClient::update_contract_type_bk_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::update_contract_type_bk_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			protocc::update_contract_type_bk_s msgs;
			try {
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("update contract_type set cycle=%d,\
interest_rate=%f,\
reserve_dispatch_coin=%d,\
rush_dispathch_coin=%d,\
rush_begin_time=%d,\
rush_end_time=%d,\
state=%d,\
name='%s',\
min_price=%d,\
max_price=%d where id=%d",
msgc.cycle(),
msgc.interest_rate(),
msgc.reserve_dispatch_coin(),
msgc.rush_dispathch_coin(),
msgc.rush_begin_time(),
msgc.rush_end_time(),
msgc.state(),
msgc.name().c_str(),
msgc.min_price(),
msgc.max_price(),
msgc.id());
				mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
				if (sr.getAffectedItemsCount() > 0) {
					msgs.set_result(0);
				}
				else {
					msgs.set_result(1);
				}
				send_proto(protocc::UPDATE_CONTRACT_TYPE_BK_S, &msgs);
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
	* 添加合约
	*/
	int BkClient::add_contract_bk_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::add_contract_bk_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			protocc::add_contract_bk_s msgs;
			try {
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("select count(phone) from `user` where phone=%llu", msgc.phone());
				pool::MysqlPool::SessionPtr ses = mysql.fetch();
				if (ses->sql(sql).execute().fetchOne().get(0).operator int() > 0) {
					msgs.set_result(0);
					for (int i = 0; i < msgc.num(); i++) {
						sql = shynet::Utility::str_format("call back_add_contract(%llu,%d,%f,4)", msgc.phone(), msgc.type(), msgc.price());
						ses->sql(sql).execute();
					}
				}
				else {
					msgs.set_result(1);
				}
				send_proto(protocc::ADD_CONTRACT_BK_S, &msgs);
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
	* 预约统计列表
	*/
	int BkClient::booking_count_bk_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		try {
			protocc::booking_count_bk_s msgs;
			pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
			std::string sql = shynet::Utility::str_format("SELECT contract_type.id,count(DISTINCT booking.phone) \
FROM contract_type LEFT JOIN booking ON booking.contract_type = contract_type.id and \
is_booking(booking.phone, booking.contract_type, contract_type.rush_begin_time, contract_type.rush_end_time) = 1 \
GROUP BY contract_type.id");
			mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
			for (auto item : sr.fetchAll()) {
				protocc::booking_count_bk_s_item* it = msgs.add_its();
				it->set_contract_type(item.get(0));
				it->set_num(item.get(1));
			}
			send_proto(protocc::BOOKING_COUNT_BK_S, &msgs);
		}
		catch (const mysqlx::Error& err) {
			LOG_WARN << err.what();
			return 0;
		}
		return 0;
	}

	/*
	* 预约明细
	*/
	int BkClient::booking_bk_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::booking_bk_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			protocc::booking_bk_s msgs;
			try {
				std::string where;
				if (msgc.phone() != 0) {
					where += "booking.phone=" + std::to_string(msgc.phone());
				}
				if (msgc.type() != 0) {
					if (where != "") {
						where += " and ";
					}
					where += "booking.contract_type=" + std::to_string(msgc.type());
				}
				if (msgc.begin_time() != "" && msgc.end_time() != "") {
					if (where != "") {
						where += " and ";
					}
					where += "booking.time BETWEEN '" + msgc.begin_time() + "' AND '" + msgc.end_time() + "'";
				}
				if (where.empty() == false) {
					where = " where " + where;
				}

				LOG_DEBUG << where;

				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("select count(phone) from booking %s", where.empty() ? "" : where.c_str());
				pool::MysqlPool::SessionPtr ses = mysql.fetch();
				int total_num = ses->sql(sql).execute().fetchOne().get(0);
				msgs.set_total_page(total_num / 10 + 1);

				sql = shynet::Utility::str_format("SELECT `user`.phone,`user`.nichen,booking.contract_type,\
UNIX_TIMESTAMP(time),booking.rob,booking.robed FROM booking \
INNER JOIN `user` on booking.phone = `user`.phone %s \
order by time desc LIMIT %d,%d ",
where.empty() ? "" : where.c_str(),
(msgc.curr_page() - 1) * 10, 10);
				mysqlx::SqlResult sr = ses->sql(sql).execute();
				for (auto item : sr.fetchAll()) {
					protocc::booking_bk_s_item* it = msgs.add_its();
					it->set_phone(item.get(0));
					it->set_nichen(item.get(1).operator std::string());
					it->set_contract_type(item.get(2));
					it->set_time(item.get(3));
					it->set_rob(item.get(4));
					it->set_robed(item.get(5));
				}
				send_proto(protocc::BOOKING_BK_S, &msgs);
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
	* 挂单统计列表
	*/
	int BkClient::ripe_count_bk_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		try {
			protocc::ripe_count_bk_s msgs;
			pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
			std::string sql = shynet::Utility::str_format("SELECT contract_type.id,count(contract_type.id) \
FROM contract_type INNER JOIN contract_info on contract_info.contract_type = contract_type.id \
LEFT JOIN user_contract on contract_info.id = user_contract.contract_id \
WHERE UNIX_TIMESTAMP(user_contract.gettime) + contract_type.cycle <= UNIX_TIMESTAMP() or user_contract.getway = 4 \
GROUP BY contract_type.id");
			mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
			for (auto item : sr.fetchAll()) {
				protocc::ripe_count_bk_s_item* it = msgs.add_its();
				it->set_contract_type(item.get(0));
				it->set_num(item.get(1));
			}
			send_proto(protocc::RIPE_COUNT_BK_S, &msgs);
		}
		catch (const mysqlx::Error& err) {
			LOG_WARN << err.what();
			return 0;
		}
		return 0;
	}

	/*
	* 挂单明细
	*/
	int BkClient::ripe_bk_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::ripe_bk_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			protocc::ripe_bk_s msgs;
			try {
				std::string where = " (UNIX_TIMESTAMP(user_contract.gettime) + contract_type.cycle <= UNIX_TIMESTAMP() or user_contract.getway = 4)";
				if (msgc.phone() != 0) {
					if (where != "") {
						where += " and ";
					}
					where += "user_contract.phone=" + std::to_string(msgc.phone());
				}
				if (msgc.type() != 0) {
					if (where != "") {
						where += " and ";
					}
					where += "contract_info.contract_type=" + std::to_string(msgc.type());
				}
				if (where.empty() == false) {
					where = " where " + where;
				}

				LOG_DEBUG << where;

				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("SELECT count(user_contract.id) FROM user_contract \
INNER JOIN contract_info ON user_contract.contract_id = contract_info.id \
INNER JOIN contract_type on contract_info.contract_type = contract_type.id \
 %s",
					where.c_str());
				pool::MysqlPool::SessionPtr ses = mysql.fetch();
				int total_num = ses->sql(sql).execute().fetchOne().get(0);
				msgs.set_total_page(total_num / 10 + 1);

				sql = shynet::Utility::str_format("SELECT user_contract.phone,\
`user`.nichen,\
contract_type.id,\
conve_contract_price(user_contract.contract_id,user_contract.phone,contract_info.current_price,contract_type.interest_rate),\
conve_contract_time(user_contract.contract_id,user_contract.phone,UNIX_TIMESTAMP(user_contract.gettime),contract_type.cycle) \
FROM user_contract \
INNER JOIN contract_info ON user_contract.contract_id = contract_info.id \
INNER JOIN `user` on user_contract.phone = `user`.phone \
INNER JOIN contract_type on contract_info.contract_type = contract_type.id \
 %s \
order by user_contract.gettime desc LIMIT %d,%d",
where.c_str(),
(msgc.curr_page() - 1) * 10, 10);
				mysqlx::SqlResult sr = ses->sql(sql).execute();
				for (auto item : sr.fetchAll()) {
					protocc::ripe_bk_s_item* it = msgs.add_its();
					it->set_phone(item.get(0));
					it->set_nichen(item.get(1).operator std::string());
					it->set_contract_type(item.get(2));
					it->set_price(item.get(3));
					it->set_time(item.get(4));
				}
				send_proto(protocc::RIPE_BK_S, &msgs);
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
	* 订单明细
	*/
	int BkClient::order_list_bk_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::order_list_bk_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			protocc::order_list_bk_s msgs;
			try {
				std::string where;
				if (msgc.buy_phone() != 0) {
					if (where != "") {
						where += " and ";
					}
					where += "buy_phone=" + std::to_string(msgc.buy_phone());
				}
				if (msgc.type() != 0) {
					if (where != "") {
						where += " and ";
					}
					where += "contract_info.contract_type=" + std::to_string(msgc.type());
				}
				if (msgc.sell_phone() != 0) {
					if (where != "") {
						where += " and ";
					}
					where += "sell_phone=" + std::to_string(msgc.sell_phone());
				}
				if (msgc.state() != 0) {
					if (where != "") {
						where += " and ";
					}
					where += "state=" + std::to_string(msgc.state());
				}
				if (msgc.id() != 0) {
					if (where != "") {
						where += " and ";
					}
					where += "id=" + std::to_string(msgc.id());
				}
				if (where.empty() == false) {
					where = " where " + where;
				}

				LOG_DEBUG << where;

				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("SELECT count(`order`.id) FROM `order` \
INNER JOIN contract_info on `order`.contract_id = contract_info.id \
INNER JOIN contract_type on contract_type.id = contract_info.contract_type \
 %s",
					where.c_str());
				pool::MysqlPool::SessionPtr ses = mysql.fetch();
				int total_num = ses->sql(sql).execute().fetchOne().get(0);
				msgs.set_total_page(total_num / 10 + 1);

				sql = shynet::Utility::str_format("SELECT `order`.id,\
`order`.buy_phone,\
`order`.sell_phone,\
contract_type.`name`,\
`order`.getway,`order`.contract_price,contract_type.interest_rate,\
`order`.payicon,\
UNIX_TIMESTAMP(`order`.rush_time),\
UNIX_TIMESTAMP(`order`.pay_time),\
UNIX_TIMESTAMP(`order`.sell_confirm_time),\
`order`.state\
 FROM `order` \
INNER JOIN contract_info on `order`.contract_id = contract_info.id \
INNER JOIN contract_type on contract_type.id = contract_info.contract_type \
 %s \
order by rush_time desc LIMIT %d,%d ",
where.c_str(),
(msgc.curr_page() - 1) * 10, 10);
				mysqlx::SqlResult sr = ses->sql(sql).execute();
				for (auto item : sr.fetchAll()) {
					protocc::order_list_bk_s_item* it = msgs.add_its();
					it->set_id(item.get(0));
					it->set_buy_phone(item.get(1));
					it->set_sell_phone(item.get(2));
					it->set_contract_type_name(item.get(3).operator std::string());
					it->set_price((float)(item.get(5).operator double()));
					it->set_payicon(item.get(7).operator std::string());
					it->set_rush_time(item.get(8).isNull() ? 0 : item.get(8).operator int());
					it->set_pay_time(item.get(9).isNull() ? 0 : item.get(9).operator int());
					it->set_sell_confirm_time(item.get(10).isNull() ? 0 : item.get(10).operator int());
					it->set_state(item.get(11));
				}
				send_proto(protocc::ORDER_LIST_BK_S, &msgs);
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
	* 订单操作
	*/
	int BkClient::order_bk_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::order_bk_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			protocc::order_bk_s msgs;
			try {
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("call back_order_oper(%lld,%d)",
					msgc.id(),
					msgc.oper());
				mysqlx::Row row = mysql.fetch()->sql(sql).execute().fetchOne();
				msgs.set_result(row.get(0));
				send_proto(protocc::ORDER_BK_S, &msgs);
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
	* 充值管理
	*/
	int BkClient::recharge_bk_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::recharge_bk_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			protocc::recharge_bk_s msgs;
			msgs.set_type(msgc.type());
			try {
				std::string where = " recharge.type=" + std::to_string(msgc.type()) +
					" and recharge.`enable`=" + std::to_string(msgc.enable()) +
					" and recharge.operator=" + std::to_string(msgc.operator_());
				if (msgc.phone() != 0) {
					if (where != "") {
						where += " and ";
					}
					where += "phone=" + std::to_string(msgc.phone());
				}

				LOG_DEBUG << "where:" << where;

				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("SELECT count(id) FROM recharge WHERE % s",
					where.c_str());
				pool::MysqlPool::SessionPtr ses = mysql.fetch();
				int total_num = ses->sql(sql).execute().fetchOne().get(0);
				msgs.set_total_page(total_num / 10 + 1);

				sql = shynet::Utility::str_format("SELECT `recharge`.id,\
`recharge`.phone,\
recharge.num,\
recharge.rmb,\
recharge.paycert,\
UNIX_TIMESTAMP(recharge.time),\
recharge.`enable`,\
recharge.operator,\
 `user`.nichen \
 FROM recharge \
INNER JOIN `user` on `user`.phone = recharge.phone \
WHERE %s \
order by recharge.time desc \
LIMIT %d,%d ",
where.c_str(),
(msgc.curr_page() - 1) * 10, 10);
				mysqlx::SqlResult sr = ses->sql(sql).execute();
				for (auto item : sr.fetchAll()) {
					protocc::recharge_bk_s_item* it = msgs.add_its();
					it->set_id(item.get(0));
					it->set_phone(item.get(1));
					it->set_num(item.get(2));
					it->set_rmb(item.get(3));
					it->set_paycert(item.get(4).operator std::string());
					it->set_time(item.get(5));
					it->set_enable(item.get(6));
					it->set_operator_(item.get(7));
					it->set_nichen(item.get(8).operator std::string());
				}
				send_proto(protocc::RECHARGE_BK_S, &msgs);
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
	* 充值操作
	*/
	int BkClient::recharge_oper_bk_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::recharge_oper_bk_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			protocc::recharge_oper_bk_s msgs;
			try {
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("call back_recharge(%d,%d,%d)",
					msgc.enable(),
					msgc.id(),
					msgc.type());
				mysqlx::Row rw = mysql.fetch()->sql(sql).execute().fetchOne();
				msgs.set_result(rw.get(0));
				if (msgs.result() == 0) {
					if (msgc.enable()) {
						//通知变化
						protocc::post_recharge_cs msgcs;
						msgcs.set_phone(rw.get(1));
						msgcs.set_type(msgc.type());
						msgcs.set_num(rw.get(2));
						AmConnector::am_connector()->send_proto(protocc::POST_RECHARGE_CS, &msgcs);
					}
				}
				send_proto(protocc::RECHARGE_OPER_BK_S, &msgs);
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
	* 用户列表
	*/
	int BkClient::user_list_bk_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::user_list_bk_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			protocc::user_list_bk_s msgs;
			try {
				std::string where = " `enable`=" + std::to_string(msgc.enable());
				if (msgc.phone() != 0) {
					if (where != "") {
						where += " and ";
					}
					where += "phone=" + std::to_string(msgc.phone());
				}

				LOG_DEBUG << "where:" << where;

				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("SELECT count(phone) FROM `user` WHERE %s",
					where.c_str());
				pool::MysqlPool::SessionPtr ses = mysql.fetch();
				int total_num = ses->sql(sql).execute().fetchOne().get(0);
				msgs.set_total_page(total_num / 10 + 1);

				sql = shynet::Utility::str_format("SELECT phone,\
nichen,\
dispatch_coin,activated_coin,\
(SELECT SUM(contract_info.current_price) FROM user_contract \
INNER JOIN contract_info ON contract_info.id = user_contract.contract_id where user_contract.phone = `user`.phone),\
static_income,\
promote_current_income,\
team_current_income,\
`activated`,\
`enable`,\
pwd,\
trade,\
parent_phone \
 FROM `user` \
WHERE %s \
order by registered_time desc \
LIMIT %d,%d ",
where.c_str(),
(msgc.curr_page() - 1) * 10, 10);
				LOG_DEBUG << sql;
				mysqlx::SqlResult sr = ses->sql(sql).execute();
				for (auto item : sr.fetchAll()) {
					protocc::user_list_bk_s_item* it = msgs.add_its();
					it->set_phone(item.get(0));
					it->set_nichen(item.get(1).operator std::string());
					it->set_dispatch_coin(item.get(2));
					it->set_activated_coin(item.get(3));
					it->set_total_income(item.get(4).isNull() ? 0.f : (float)(item.get(4).operator double()));
					it->set_static_income(item.get(5));
					it->set_promote_current_income(item.get(6));
					it->set_team_current_income(item.get(7));
					it->set_activated(item.get(8));
					it->set_enable(item.get(9));
					it->set_pwd(item.get(10).isNull() ? "" : item.get(10).operator std::string());
					it->set_trade(item.get(11).isNull() ? 0 : item.get(11).operator int());
					it->set_parent_phone(item.get(12));
				}
				send_proto(protocc::USER_LIST_BK_S, &msgs);
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
	* 编辑用户
	*/
	int BkClient::update_user_bk_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::update_user_bk_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			protocc::update_user_bk_s msgs;
			try {
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("update `user` set dispatch_coin=%d,\
activated_coin=%d,\
promote_current_income=%d,\
team_current_income=%d,\
pwd='%s',\
trade=%d,\
parent_phone=%llu \
where phone=%llu",
msgc.dispatch_coin(),
msgc.activated_coin(),
msgc.promote_current_income(),
msgc.team_current_income(),
msgc.pwd().c_str(),
msgc.trade(),
msgc.parent_phone(),
msgc.phone());
				mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
				if (sr.getAffectedItemsCount() > 0) {
					msgs.set_result(0);
				}
				else {
					msgs.set_result(1);
				}
				send_proto(protocc::UPDATE_USER_BK_S, &msgs);
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
	* 玩家合约列表
	*/
	int BkClient::user_contract_list_bk_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::update_user_bk_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			try {
				protocc::user_contract_list_bk_s msgs;
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("SELECT user_contract.id, \
contract_type.`id`,\
contract_info.current_price,\
conve_contract_time(user_contract.contract_id,user_contract.phone,UNIX_TIMESTAMP(user_contract.gettime),contract_type.cycle) \
FROM user_contract INNER JOIN contract_info on user_contract.contract_id = contract_info.id \
INNER JOIN contract_type on contract_type.id = contract_info.contract_type WHERE phone = %llu",
msgc.phone());
				mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
				for (auto item : sr.fetchAll()) {
					protocc::user_contract_list_bk_s_item* it = msgs.add_its();
					it->set_id(item.get(0));
					it->set_contract_type(item.get(1));
					it->set_price(item.get(2));
					it->set_time(item.get(3));
				}
				send_proto(protocc::USER_CONTRACT_LIST_BK_S, &msgs);
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
	* 删除合约
	*/
	int BkClient::del_user_contract_bk_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::del_user_contract_bk_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			protocc::del_user_contract_bk_s msgs;
			try {
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("DELETE FROM user_contract WHERE id=%d", msgc.id());
				mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
				if (sr.getAffectedItemsCount() > 0) {
					msgs.set_result(0);
				}
				else {
					msgs.set_result(1);
				}
				send_proto(protocc::DEL_USER_CONTRACT_BK_S, &msgs);
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
	* 实名认证列表
	*/
	int BkClient::name_verifie_list_bk_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::name_verifie_list_bk_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			protocc::name_verifie_list_bk_s msgs;
			try {
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("SELECT count(phone) FROM `user` where name_verifie <> 1");
				pool::MysqlPool::SessionPtr ses = mysql.fetch();
				int total_num = ses->sql(sql).execute().fetchOne().get(0);
				msgs.set_total_page(total_num / 10 + 1);

				sql = shynet::Utility::str_format("SELECT phone,\
nichen,\
activated,\
`name`,\
id_card,\
name_verifie\
 FROM `user` where name_verifie <> 1  \
order by name_verifie_back asc, registered_time desc \
LIMIT %d,%d ",
(msgc.curr_page() - 1) * 10, 10);
				mysqlx::SqlResult sr = ses->sql(sql).execute();
				for (auto item : sr.fetchAll()) {
					protocc::name_verifie_list_bk_s_item* it = msgs.add_its();
					it->set_phone(item.get(0));
					it->set_nichen(item.get(1).operator std::string());
					it->set_activated(item.get(2));
					it->set_name(item.get(3).operator std::string());
					it->set_idcard(item.get(4).operator std::string());
					it->set_name_verifie(item.get(5));
				}
				send_proto(protocc::NAME_VERIFIE_LIST_BK_S, &msgs);
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
	* 实名认证操作
	*/
	int BkClient::name_verifie_oper_bk_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::name_verifie_oper_bk_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			protocc::name_verifie_oper_bk_s msgs;
			try {
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("update `user` set name_verifie_back=%d,name_verifie=%d where phone=%lld",
					msgc.type(),
					msgc.type() == 1 ? 3 : 1,
					msgc.phone());
				mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
				if (sr.getAffectedItemsCount() > 0) {
					msgs.set_result(0);
					//通知变化
					protocc::post_verifie_cs msgcs;
					msgcs.set_phone(msgc.phone());
					msgcs.set_type(1);
					msgcs.set_result(msgc.type());
					AmConnector::am_connector()->send_proto(protocc::POST_VERIFIE_CS, &msgcs);
				}
				else {
					msgs.set_result(1);
				}
				send_proto(protocc::NAME_VERIFIE_OPER_BK_S, &msgs);
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
	* 支付认证列表
	*/
	int BkClient::pay_verifie_list_bk_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::pay_verifie_list_bk_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			protocc::pay_verifie_list_bk_s msgs;
			try {
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("SELECT count(phone) FROM `user` where pay_verifie <> 1");
				pool::MysqlPool::SessionPtr ses = mysql.fetch();
				int total_num = ses->sql(sql).execute().fetchOne().get(0);
				msgs.set_total_page(total_num / 10 + 1);

				sql = shynet::Utility::str_format("SELECT phone,\
nichen,\
activated,\
bank_name,\
bank_cardid,\
bank_user_name,\
ailpay_id,\
alipay_name,\
webchat_name,\
pay_verifie\
 FROM `user` \
 where pay_verifie <> 1 \
order by pay_verifie_back asc, registered_time desc \
LIMIT %d,%d ",
(msgc.curr_page() - 1) * 10, 10);
				mysqlx::SqlResult sr = ses->sql(sql).execute();
				for (auto item : sr.fetchAll()) {
					protocc::pay_verifie_list_bk_s_item* it = msgs.add_its();
					it->set_phone(item.get(0));
					it->set_nichen(item.get(1).operator std::string());
					it->set_activated(item.get(2));
					it->set_bank_name(item.get(3).operator std::string());
					it->set_bank_cardid(item.get(4).operator std::string());
					it->set_bank_user_name(item.get(5).operator std::string());
					it->set_ailpay_id(item.get(6).operator std::string());
					it->set_alipay_name(item.get(7).operator std::string());
					it->set_webchat_name(item.get(8).operator std::string());
					it->set_pay_verifie(item.get(9));
				}
				send_proto(protocc::PAY_VERIFIE_LIST_BK_S, &msgs);
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
	* 支付认证操作
	*/
	int BkClient::pay_verifie_oper_bk_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::pay_verifie_oper_bk_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			protocc::pay_verifie_oper_bk_s msgs;
			try {
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("update `user` set pay_verifie_back=%d,pay_verifie=%d where phone=%lld",
					msgc.type(),
					msgc.type() == 1 ? 3 : 1,
					msgc.phone());
				mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
				if (sr.getAffectedItemsCount() > 0) {
					msgs.set_result(0);
					//通知变化
					protocc::post_verifie_cs msgcs;
					msgcs.set_phone(msgc.phone());
					msgcs.set_type(2);
					msgcs.set_result(msgc.type());
					AmConnector::am_connector()->send_proto(protocc::POST_VERIFIE_CS, &msgcs);
				}
				else {
					msgs.set_result(1);
				}
				send_proto(protocc::PAY_VERIFIE_OPER_BK_S, &msgs);
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
	* 生成用户
	*/
	int BkClient::create_user_bk_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::create_user_bk_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			protocc::create_user_bk_s msgs;
			try {
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("call create_user(%llu,'%s','%s',%d)",
					msgc.phone(),
					msgc.nichen().c_str(),
					msgc.pwd().c_str(),
					msgc.trade());
				mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
				mysqlx::Row row = sr.fetchOne();
				msgs.set_result(row.get(0));
				send_proto(protocc::CREATE_USER_BK_S, &msgs);
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
	* 玩家代理明细
	*/
	int BkClient::user_proxy_bk_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::user_proxy_bk_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			protocc::user_proxy_bk_s msgs;
			try {
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("call back_user_proxy(%llu)",
					msgc.phone());
				mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
				mysqlx::Row row = sr.fetchOne();
				msgs.set_phone(row.get(0));
				msgs.set_nichen(row.get(1).operator std::string());
				msgs.set_parent_phone(row.get(2));
				msgs.set_parent_nichen(row.get(3).operator std::string());
				msgs.set_sum_1_activated(row.get(4));
				msgs.set_count_1_phone(row.get(5));
				msgs.set_sum_2_activated(row.get(6));
				msgs.set_count_2_phone(row.get(7));
				msgs.set_sum_3_activated(row.get(8));
				msgs.set_count_3_phone(row.get(9));
				msgs.set_sum_99_activated(row.get(10));
				msgs.set_count_99_phone(row.get(11));
				msgs.set_team_total_income(row.get(12));
				send_proto(protocc::USER_PROXY_BK_S, &msgs);
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
	* 封号
	*/
	int BkClient::close_user_bk_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::close_user_bk_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			protocc::close_user_bk_s msgs;
			try {
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				std::string sql = shynet::Utility::str_format("UPDATE `user` SET `enable` = 1,disenbale_time = now() WHERE phone = %llu",
					msgc.phone());
				mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
				if (sr.getAffectedItemsCount() > 0) {
					msgs.set_result(0);
				}
				else {
					msgs.set_result(1);
				}
				send_proto(protocc::CLOSE_USER_BK_S, &msgs);
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
	* 有新的充值
	*/
	int BkClient::new_recharge_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::new_recharge_s msgs;
		try {
			pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
			std::string sql = shynet::Utility::str_format("select count(id) from recharge where operator = 0");
			mysqlx::Row row = mysql.fetch()->sql(sql).execute().fetchOne();
			if (row.get(0).operator int() > 0) {
				msgs.set_result(0);
			}
			else {
				msgs.set_result(1);
			}
			send_proto(protocc::NEW_RECHARGE_S, &msgs);
		}
		catch (const mysqlx::Error& err) {
			LOG_WARN << err.what();
			return 0;
		}
		return 0;
	}

	/*
	* 合约升级拆分列表
	*/
	int BkClient::contract_oper_list_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::contract_oper_list_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			protocc::contract_oper_list_s msgs;
			msgs.set_type(msgc.type());
			try {
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				if (msgc.type() == 1) {
					//升级
					std::string sql = shynet::Utility::str_format("SELECT contract_type.id,contract_info.current_price,user_contract.phone,`user`.nichen \
FROM user_contract \
INNER JOIN `user` on `user`.phone = user_contract.phone \
INNER JOIN contract_info on user_contract.contract_id = contract_info.id \
INNER JOIN contract_type on contract_type.id = contract_info.contract_type \
WHERE (UNIX_TIMESTAMP(user_contract.gettime) + contract_type.cycle <= UNIX_TIMESTAMP() or user_contract.getway = 4) \
and contract_info.current_price > contract_type.max_price");
					mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
					for (auto item : sr.fetchAll()) {
						protocc::contract_oper_list_s_item* it = msgs.add_its();
						it->set_contract_type(item.get(0));
						it->set_price(item.get(1).operator float());
						it->set_phone(item.get(2));
						it->set_nichen(item.get(3).operator std::string());
					}
				}
				else {
					//拆分
					std::string sql = shynet::Utility::str_format("SELECT contract_type.id,contract_info.current_price,user_contract.phone,`user`.nichen \
FROM user_contract \
INNER JOIN `user` on `user`.phone = user_contract.phone \
INNER JOIN contract_info on user_contract.contract_id = contract_info.id \
INNER JOIN contract_type on contract_type.id = contract_info.contract_type \
WHERE (UNIX_TIMESTAMP(user_contract.gettime) + contract_type.cycle <= UNIX_TIMESTAMP() or user_contract.getway = 4) \
and contract_type.id = 6 \
and contract_info.current_price > contract_type.max_price");
					mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
					for (auto item : sr.fetchAll()) {
						protocc::contract_oper_list_s_item* it = msgs.add_its();
						it->set_contract_type(item.get(0));
						it->set_price(item.get(1).operator float());
						it->set_phone(item.get(2));
						it->set_nichen(item.get(3).operator std::string());
					}
				}
				send_proto(protocc::CONTRACT_OPER_LIST_S, &msgs);
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
	* 合约升级
	*/
	int BkClient::contract_oper_c(std::shared_ptr<protocc::CommonObject> data, std::stack<protocol::FilterProces::Envelope>& enves) {
		protocc::contract_oper_c msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			protocc::contract_oper_s msgs;
			msgs.set_type(msgc.type());
			try {
				pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
				if (msgc.type() == 1) {
					//升级
					std::string sql = shynet::Utility::str_format("SELECT contract_info.id,contract_info.current_price \
FROM user_contract \
INNER JOIN contract_info on user_contract.contract_id = contract_info.id \
INNER JOIN contract_type on contract_type.id = contract_info.contract_type \
WHERE (UNIX_TIMESTAMP(user_contract.gettime) + contract_type.cycle <= UNIX_TIMESTAMP() or user_contract.getway = 4) \
and contract_info.current_price > contract_type.max_price");
					mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
					for (auto item : sr.fetchAll()) {
						int contract_type = item.get(0);
						float price = item.get(1).operator float();
						sql = shynet::Utility::str_format("call back_contract_upgrade(%d,%f)", contract_type, price);
						mysql.fetch()->sql(sql).execute();
					}
				}
				else {
					//拆分
					std::string sql = shynet::Utility::str_format("SELECT user_contract.id \
FROM user_contract \
INNER JOIN contract_info on user_contract.contract_id = contract_info.id \
INNER JOIN contract_type on contract_type.id = contract_info.contract_type \
WHERE (UNIX_TIMESTAMP(user_contract.gettime) + contract_type.cycle <= UNIX_TIMESTAMP() or user_contract.getway = 4) \
and contract_type.id = 6 \
and contract_info.current_price > contract_type.max_price");
					mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
					for (auto item : sr.fetchAll()) {
						sql = shynet::Utility::str_format("call back_contract_spilt(%d)", item.get(0).operator int());
						mysql.fetch()->sql(sql).execute();
					}
				}
				send_proto(protocc::CONTRACT_OPER_S, &msgs);
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
