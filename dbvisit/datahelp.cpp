#include "dbvisit/datahelp.h"
#include "3rd/rapidjson/document.h"
#include "3rd/rapidjson/stringbuffer.h"
#include "3rd/rapidjson/writer.h"
#include "3rd/sw/redis++/redis++.h"
#include "dbvisit/datatimer.h"
#include "dbvisit/datatimermgr.h"
#include "shynet/net/timerreactormgr.h"
#include "shynet/pool/mysqlpool.h"
#include "shynet/utils/stringop.h"
namespace redis = sw::redis;
#include <unordered_set>

namespace dbvisit {

Datahelp::ErrorCode Datahelp::getdata_from_db(const std::string& tablename,
    const std::string& key, std::unordered_map<std::string, std::string>& out,
    const std::string& where)
{
    pool::MysqlPool& mysql = shynet::utils::Singleton<pool::MysqlPool>::get_instance();
    std::string sql = shynet::utils::stringop::str_format("_id='%s'", key.c_str());
    std::string condition = sql;
    if (key.empty()) {
        condition = where;
    }
    pool::MysqlPool::SessionPtr ses = mysql.fetch();
    mysqlx::DocResult docs = ses->getDefaultSchema().createCollection(tablename, true).find(condition).execute();
    if (docs.count() == 1) {
        mysqlx::DbDoc doc = docs.fetchOne();
        //填充数据
        for (auto&& [key, value] : out) {
            if (doc.hasField(key)) {
                value = doc[key].operator std::string();
            }
        }
    } else if (docs.count() > 1) {
        LOG_WARN << "db数据不唯一 tablename:" << tablename << " key:" << key << " where:" + where;
    } else {
        return ErrorCode::NOT_DATA;
    }
    return ErrorCode::OK;
}

Datahelp::ErrorCode Datahelp::getdata_from_cache(const std::string& cachekey,
    std::unordered_map<std::string, std::string>& out,
    std::chrono::seconds seconds)
{

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
    } else {
        return ErrorCode::NOT_DATA;
    }
    return ErrorCode::OK;
}

Datahelp::ErrorCode Datahelp::getdata(const std::string& cachekey,
    std::unordered_map<std::string, std::string>& out,
    OperType opertype,
    bool updatacache,
    std::chrono::seconds seconds)
{
    ErrorCode error = ErrorCode::OK;
    if (opertype == OperType::ALL) {

        error = getdata_from_cache(cachekey, out);
        if (error == ErrorCode::NOT_DATA) {
        db_label:
            const auto temp = shynet::utils::stringop::split(cachekey, "_");
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
    } else if (opertype == OperType::CACHE) {
        return getdata_from_cache(cachekey, out);
    } else if (opertype == OperType::DB) {
        goto db_label;
    }
    return ErrorCode::NOT_DATA;
}

moredataptr Datahelp::getdata_more_db(const std::string& tablename,
    const std::string& condition,
    std::unordered_map<std::string, std::string>& out,
    std::string sort,
    int32_t limit)
{
    moredataptr datalist = std::make_shared<moredata>();
    pool::MysqlPool& mysql = shynet::utils::Singleton<pool::MysqlPool>::get_instance();
    pool::MysqlPool::SessionPtr ses = mysql.fetch();
    mysqlx::abi2::r0::Collection coll = ses->getDefaultSchema().createCollection(tablename, true);
    auto cf = condition.empty() ? coll.find() : coll.find(condition);
    if (sort.empty() == false) {
        cf = cf.sort(sort);
    }
    mysqlx::DocResult docs;
    if (limit == 0) {
        docs = cf.execute();
    } else {
        docs = cf.limit(limit).execute();
    }
    for (mysqlx::DbDoc doc : docs.fetchAll()) {
        //填充数据
        std::unordered_map<std::string, std::string> outcopy = out;
        for (auto&& [key, value] : outcopy) {
            if (doc.hasField(key)) {
                value = doc[key].operator std::string();
            }
        }
        datalist->push_back(outcopy);
    }
    return datalist;
}

moredataptr Datahelp::getdata_more_cache(const std::string& condition,
    std::unordered_map<std::string, std::string>& out,
    std::string sort,
    int32_t limit)
{

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
    if (sort.empty() == false) {
        const auto temp = shynet::utils::stringop::split(sort, " ");
        if (temp.size() < 2) {
            THROW_EXCEPTION("解析错误 sort:" + sort);
        }
        datalist->sort([&temp, &sort](const std::unordered_map<std::string, std::string>& t1, std::unordered_map<std::string, std::string>& t2) {
            auto it1 = t1.find(temp[0]);
            auto it2 = t2.find(temp[0]);
            if (it1 == t1.end() || it2 == t2.end()) {
                THROW_EXCEPTION("解析错误 sort key:" + sort);
            }
            if (temp[1] == "asc") {
                return it1->second < it2->second;
            } else {
                return it1->second > it2->second;
            }
        });
    }
    if (limit != 0) {
        while (datalist->size() > size_t(limit)) {
            datalist->erase(--datalist->end());
        }
    }
    return datalist;
}

moredataptr Datahelp::getdata_more(const std::string& condition,
    std::unordered_map<std::string, std::string>& out,
    std::string sort,
    int32_t limit,
    OperType opertype,
    bool updatacache,
    std::chrono::seconds seconds)
{
    moredataptr datalist;
    if (opertype == OperType::ALL) {
        datalist = getdata_more_cache(condition, out, sort, limit);
        if (datalist->empty()) {
        db_label:
            auto vect = shynet::utils::stringop::split(condition, "_");
            if (vect.size() < 3) {
                THROW_EXCEPTION("解析错误 condition:" + condition);
            }
            std::string where;
            if (vect[2] != "*")
                where = shynet::utils::stringop::str_format("roleid='%s'", vect[2].c_str());
            datalist = getdata_more_db(vect[0], where, out, sort, limit);
            if (updatacache && opertype == OperType::ALL) {
                for (auto& it : *datalist) {
                    std::string cache_key = condition;
                    auto pos = cache_key.find("*");
                    if (pos == std::string::npos)
                        THROW_EXCEPTION("找不到* condition:" + condition);
                    cache_key.replace(pos, 1, it["_id"]);
                    redis::Redis& redis = shynet::utils::Singleton<redis::Redis>::instance(std::string());
                    redis.hmset(cache_key, it.begin(), it.end());
                    if (seconds.count() != 0) {
                        redis.expire(cache_key, seconds);
                    }
                }
            }
        }
    } else if (opertype == OperType::CACHE) {
        datalist = getdata_more_cache(condition, out, sort, limit);
    } else if (opertype == OperType::DB) {
        goto db_label;
    }

    return datalist;
}

void Datahelp::insert_db(const std::string& tablename, const std::string& key,
    const std::unordered_map<std::string, std::string>& fields)
{
    pool::MysqlPool& mysql = shynet::utils::Singleton<pool::MysqlPool>::get_instance();
    pool::MysqlPool::SessionPtr ses = mysql.fetch();
    mysqlx::Schema sch = ses->getDefaultSchema();
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
    if (sch.createCollection(tablename, true).add(buffer.GetString()).execute().getAffectedItemsCount() == 0) {
        LOG_WARN << "数据插入失败 tablename:" << tablename << " key:" << key;
    }
}

void Datahelp::insert_cache(const std::string& cachekey, const std::unordered_map<std::string, std::string>& fields, std::chrono::seconds seconds)
{
    redis::Redis& redis = shynet::utils::Singleton<redis::Redis>::instance(std::string());
    redis.hmset(cachekey, fields.begin(), fields.end());
    if (seconds.count() != 0) {
        redis.expire(cachekey, seconds);
    }
}

void Datahelp::insertdata(const std::string& cachekey,
    const std::unordered_map<std::string, std::string>& fields,
    OperType opertype,
    std::chrono::seconds seconds)
{

    if (opertype == OperType::ALL) {
        insert_cache(cachekey, fields, seconds);

    db_label:
        std::vector<std::string> temp = shynet::utils::stringop::split(cachekey, "_");
        if (temp.size() < 2) {
            THROW_EXCEPTION("解析错误 cachekey:" + cachekey);
            return;
        }
        return insert_db(temp[0], temp[1], fields);
    } else if (opertype == OperType::CACHE) {
        insert_cache(cachekey, fields, seconds);
    } else if (opertype == OperType::DB) {
        goto db_label;
    }
}

void Datahelp::delete_db(const std::string& tablename, const std::string& key)
{
    pool::MysqlPool& mysql = shynet::utils::Singleton<pool::MysqlPool>::get_instance();
    pool::MysqlPool::SessionPtr ses = mysql.fetch();
    mysqlx::Schema sch = ses->getDefaultSchema();
    std::string sql = shynet::utils::stringop::str_format("_id='%s'", key.c_str());
    if (sch.createCollection(tablename, true).remove(sql).execute().getAffectedItemsCount() == 0) {
        LOG_WARN << "数据删除失败 tablename:" << tablename << " key:" << key;
    }
}

void Datahelp::delete_cache(const std::string& cachekey)
{
    redis::Redis& redis = shynet::utils::Singleton<redis::Redis>::instance(std::string());
    long long ret = redis.del(cachekey);
    if (ret == 0) {
        LOG_WARN << "cachekey数据删除失败 cachekey:" << cachekey;
    }
}

void Datahelp::deletedata(const std::string& cachekey, OperType opertype)
{
    if (opertype == OperType::ALL) {
        delete_cache(cachekey);

    db_label:
        std::vector<std::string> temp = shynet::utils::stringop::split(cachekey, "_");
        if (temp.size() < 2) {
            THROW_EXCEPTION("解析错误 cachekey:" + cachekey);
            return;
        }
        delete_db(temp[0], temp[1]);
    } else if (opertype == OperType::CACHE) {
        delete_cache(cachekey);
    } else if (opertype == OperType::DB) {
        goto db_label;
    }
}

void Datahelp::updata_db(const std::string& tablename, const std::string& key, const std::unordered_map<std::string, std::string>& fields)
{

    pool::MysqlPool& mysql = shynet::utils::Singleton<pool::MysqlPool>::get_instance();
    pool::MysqlPool::SessionPtr ses = mysql.fetch();
    mysqlx::Schema sch = ses->getDefaultSchema();
    std::string where = shynet::utils::stringop::str_format("_id='%s'", key.c_str());
    mysqlx::CollectionModify md = sch.createCollection(tablename, true).modify(where);
    for (auto&& [key, value] : fields) {
        if (key != "_id") {
            md.set(key, value);
        }
    }
    if (md.execute().getAffectedItemsCount() == 0) {
        //LOG_WARN << "数据更新失败 tablename:" << tablename << " key:" << key;
    }
}

void Datahelp::updata_cache(const std::string& cachekey,
    const std::unordered_map<std::string, std::string>& fields,
    std::chrono::seconds seconds)
{
    redis::Redis& redis = shynet::utils::Singleton<redis::Redis>::instance(std::string());
    redis.hmset(cachekey, fields.begin(), fields.end());
    if (seconds.count() != 0) {
        redis.expire(cachekey, seconds);
    }
}

void Datahelp::updata(const std::string& cachekey,
    const std::unordered_map<std::string, std::string>& fields,
    OperType opertype,
    bool immediately,
    const timeval val,
    std::chrono::seconds seconds)
{
    if (opertype == OperType::ALL) {
        updata_cache(cachekey, fields, seconds);

    db_label:
        std::vector<std::string> temp = shynet::utils::stringop::split(cachekey, "_");
        if (temp.size() < 2) {
            THROW_EXCEPTION("解析错误 cachekey:" + cachekey);
            return;
        }
        if (immediately) {
            //立即写库
            return updata_db(temp[0], temp[1], fields);
        } else {
            //延时排队写库
            auto timerid = shynet::utils::Singleton<DataTimerMgr>::instance().find(cachekey);
            if (timerid == 0) {
                //创建写库计时器
                std::shared_ptr<DataTimer> dt = std::make_shared<DataTimer>(cachekey, val);
                dt->modify_cache_fields(fields);
                shynet::utils::Singleton<DataTimerMgr>::instance().add(cachekey,
                    shynet::utils::Singleton<shynet::net::TimerReactorMgr>::instance().add(dt));
            } else {
                //延迟写库计时器执行时间
                std::shared_ptr<DataTimer> dt = std::dynamic_pointer_cast<DataTimer>(
                    shynet::utils::Singleton<shynet::net::TimerReactorMgr>::instance().find(timerid));
                if (dt) {
                    dt->set_val(val);
                    dt->modify_cache_fields(fields);
                }
            }
        }
    } else if (opertype == OperType::CACHE) {
        updata_cache(cachekey, fields, seconds);
    } else {
        goto db_label;
    }
}
}
