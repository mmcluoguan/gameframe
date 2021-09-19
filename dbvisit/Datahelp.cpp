#include "dbvisit/Datahelp.h"
#include "dbvisit/DataTimerMgr.h"
#include "dbvisit/DataTimer.h"
#include "shynet/net/TimerReactorMgr.h"
#include "shynet/pool/MysqlPool.h"
#include "shynet/utils/StringOp.h"
#include <sw/redis++/redis++.h>
namespace redis = sw::redis;
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <unordered_set>

namespace dbvisit {
	Datahelp::Datahelp() {
	}

	Datahelp::ErrorCode Datahelp::getdata_from_db(const std::string& tablename,
		const std::string& key, std::unordered_map<std::string, std::string>& out,
		const std::string& where) {
		pool::MysqlPool& mysql = shynet::utils::Singleton<pool::MysqlPool>::get_instance();
		std::string sql = shynet::utils::StringOp::str_format("_id='%s'", key.c_str());
		std::string condition = sql;
		if (key.empty()) {
			condition = where;
		}
		mysqlx::DocResult docs = mysql.fetch()->getDefaultSchema().createCollection(tablename, true).find(condition).execute();
		if (docs.count() == 1) {
			mysqlx::DbDoc doc = docs.fetchOne();
			//填充数据
			for (auto&& [key, value] : out) {
				if (doc.hasField(key)) {
					value = doc[key].operator std::string();
				}
			}
		}
		else if (docs.count() > 1) {
			LOG_WARN << "db数据不唯一 tablename:" << tablename << " key:" << key << " where:" + where;
		}
		else {
			return ErrorCode::NOT_DATA;
		}
		return ErrorCode::OK;
	}

	Datahelp::ErrorCode Datahelp::getdata_from_cache(const std::string& cachekey,
		std::unordered_map<std::string, std::string>& out,
		std::chrono::seconds seconds) {

		redis::Redis& redis = shynet::utils::Singleton<redis::Redis>::instance(std::string());
		if (redis.exists(cachekey) == 1) {
			std::vector<std::string> out_key;
			for (auto&& [key, value] : out) {
				out_key.push_back(key);
			}
			std::vector<redis::OptionalString> out_value;
			redis.hmget(cachekey, out_key.begin(), out_key.end(), std::back_inserter(out_value));
			for (size_t i = 0; i < out_key.size(); ++i) {
				const redis::OptionalString& value = out_value[i];
				if (value) {
					out[out_key[i]] = *value;
				}
			}
			if (seconds.count() != 0) {
				redis.expire(cachekey, seconds);
			}
		}
		else {
			return ErrorCode::NOT_DATA;
		}
		return ErrorCode::OK;

	}

	Datahelp::ErrorCode Datahelp::getdata(const std::string& cachekey,
		std::unordered_map<std::string, std::string>& out,
		bool updatacache,
		std::chrono::seconds seconds) {
		ErrorCode error = getdata_from_cache(cachekey, out);
		if (error == ErrorCode::NOT_DATA) {
			const auto temp = shynet::utils::StringOp::split(cachekey, "_");
			if (temp.size() < 2) {
				THROW_EXCEPTION("解析错误 cachekey:" + cachekey);
			}
			error = getdata_from_db(temp[0], temp[1], out);
			if (error == ErrorCode::OK) {
				if (updatacache) {
					redis::Redis& redis = shynet::utils::Singleton<redis::Redis>::instance(std::string());
					redis.hset(cachekey, out.begin(), out.end());
					if (seconds.count() != 0) {
						redis.expire(cachekey, seconds);
					}
				}
			}
		}
		return error;
	}

	moredataptr Datahelp::getdata_more_cache(const std::string& condition,
		std::unordered_map<std::string, std::string>& out) {

		moredataptr datalist = std::make_shared<moredata>();
		redis::Redis& redis = shynet::utils::Singleton<redis::Redis>::instance(std::string());
		auto cursor = 0LL;
		std::unordered_set<std::string> keys;
		while (true) {
			cursor = redis.scan(cursor, condition, 50, std::inserter(keys, keys.begin()));
			if (cursor == 0) {
				break;
			}
		}
		for (auto& key : keys) {
			ErrorCode err = getdata_from_cache(key, out);
			if (err == ErrorCode::OK) {
				datalist->push_back(out);
			}
		}
		return datalist;
	}

	void Datahelp::updata_db(const std::string& tablename, const std::string& key, const std::unordered_map<std::string, std::string>& fields) {

		pool::MysqlPool& mysql = shynet::utils::Singleton<pool::MysqlPool>::get_instance();
		mysqlx::Schema sch = mysql.fetch()->getDefaultSchema();
		std::string where = shynet::utils::StringOp::str_format("_id='%s'", key.c_str());
		mysqlx::CollectionModify md = sch.createCollection(tablename, true).modify(where);
		for (auto&& [key, value] : fields) {
			if (key != "_id") {
				md.set(key, value);
			}
		}
		if (md.execute().getAffectedItemsCount() == 0) {
			LOG_WARN << "数据更新失败 tablename:" << tablename << " key:" << key;
		}
	}

	void Datahelp::insert_db(const std::string& tablename, const std::string& key,
		const std::unordered_map<std::string, std::string>& fields) {
		pool::MysqlPool& mysql = shynet::utils::Singleton<pool::MysqlPool>::get_instance();
		mysqlx::Schema sch = mysql.fetch()->getDefaultSchema();
		rapidjson::Document doc;
		rapidjson::Value& data = doc.SetObject();
		for (auto&& [key, value] : fields) {
			data.AddMember(rapidjson::StringRef(key.c_str()),
				rapidjson::StringRef(value.c_str()),
				doc.GetAllocator());
		}
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		data.Accept(writer);
		if (sch.createCollection(tablename, true).add(buffer.GetString())
			.execute().getAffectedItemsCount() == 0) {
			LOG_WARN << "数据插入失败 tablename:" << tablename << " key:" << key;
		}
	}

	void Datahelp::delete_db(const std::string& tablename, const std::string& key) {
		pool::MysqlPool& mysql = shynet::utils::Singleton<pool::MysqlPool>::get_instance();
		mysqlx::Schema sch = mysql.fetch()->getDefaultSchema();
		std::string sql = shynet::utils::StringOp::str_format("_id='%s'", key.c_str());
		if (sch.createCollection(tablename, true).remove(sql)
			.execute().getAffectedItemsCount() == 0) {
			LOG_WARN << "数据删除失败 tablename:" << tablename << " key:" << key;
		}
	}

	void Datahelp::insertdata(const std::string& cachekey,
		const std::unordered_map<std::string, std::string>& fields,
		std::chrono::seconds seconds) {

		redis::Redis& redis = shynet::utils::Singleton<redis::Redis>::instance(std::string());
		redis.hmset(cachekey, fields.begin(), fields.end());
		if (seconds.count() != 0) {
			redis.expire(cachekey, seconds);
		}
		std::vector<std::string> temp = shynet::utils::StringOp::split(cachekey, "_");
		if (temp.size() < 2) {
			THROW_EXCEPTION("解析错误 cachekey:" + cachekey);
			return;
		}
		//立即写库				
		return insert_db(temp[0], temp[1], fields);
	}

	void Datahelp::deletedata(const std::string& cachekey) {
		redis::Redis& redis = shynet::utils::Singleton<redis::Redis>::instance(std::string());
		long long ret = redis.del(cachekey);
		if (ret == 0) {
			LOG_WARN << "cachekey数据删除失败 cachekey:" << cachekey;
		}
		else {
			std::vector<std::string> temp = shynet::utils::StringOp::split(cachekey, "_");
			if (temp.size() < 2) {
				THROW_EXCEPTION("解析错误 cachekey:" + cachekey);
				return;
			}
			delete_db(temp[0], temp[1]);
		}
	}

	void Datahelp::updata_cache(const std::string& cachekey,
		const std::unordered_map<std::string, std::string>& fields,
		std::chrono::seconds seconds) {
		redis::Redis& redis = shynet::utils::Singleton<redis::Redis>::instance(std::string());
		redis.hmset(cachekey, fields.begin(), fields.end());
		if (seconds.count() != 0) {
			redis.expire(cachekey, seconds);
		}
	}

	void Datahelp::updata(const std::string& cachekey,
		const std::unordered_map<std::string, std::string>& fields,
		bool immediately,
		const timeval val,
		std::chrono::seconds seconds) {

		redis::Redis& redis = shynet::utils::Singleton<redis::Redis>::instance(std::string());
		redis.hmset(cachekey, fields.begin(), fields.end());
		if (seconds.count() != 0) {
			redis.expire(cachekey, seconds);
		}
		std::vector<std::string> temp = shynet::utils::StringOp::split(cachekey, "_");
		if (temp.size() < 2) {
			THROW_EXCEPTION("解析错误 cachekey:" + cachekey);
			return;
		}
		if (immediately) {
			//立即写库				
			return updata_db(temp[0], temp[1], fields);
		}
		else {
			//延时排队写库				
			auto timerid = shynet::utils::Singleton<DataTimerMgr>::instance().find(cachekey);
			if (timerid == 0) {
				//创建写库计时器
				std::shared_ptr<DataTimer> dt = std::make_shared<DataTimer>(cachekey, val);
				dt->modify_cache_fields(fields);
				shynet::utils::Singleton<DataTimerMgr>::instance().add(cachekey,
					shynet::utils::Singleton<shynet::net::TimerReactorMgr>::instance().add(dt));
			}
			else {
				//延迟写库计时器执行时间
				std::shared_ptr<DataTimer> dt = std::dynamic_pointer_cast<DataTimer>(
					shynet::utils::Singleton<shynet::net::TimerReactorMgr>::instance().find(timerid));
				if (dt) {
					dt->set_val(val);
					dt->modify_cache_fields(fields);
				}
			}
		}
	}
}
