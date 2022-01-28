#include "dbvisit/dbclient.h"
#include "3rd/fmt/format.h"
#include "dbvisit/datahelp.h"
#include "dbvisit/dbclientmgr.h"
#include "frmpub/luacallbacktask.h"
#include "shynet/lua/luaengine.h"
#include "shynet/pool/mysqlpool.h"
#include "shynet/utils/elapsed.h"
#include "shynet/utils/idworker.h"
#include "shynet/utils/stringop.h"

namespace dbvisit {
DbClient::DbClient(std::shared_ptr<net::IPAddress> remote_addr,
    std::shared_ptr<net::IPAddress> listen_addr,
    std::shared_ptr<events::EventBuffer> iobuf)
    : frmpub::Client(remote_addr, listen_addr, iobuf)
{
    LOG_INFO << "新客户端连接 [ip:" << remote_addr->ip() << ":" << remote_addr->port() << "]";

    pmb_ = {
        { protocc::ERRCODE,
            std::bind(&DbClient::errcode, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::REGISTER_WORLD_DBVISIT_C,
            std::bind(&DbClient::register_world_dbvisit_c, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::REGISTER_LOGIN_DBVISIT_C,
            std::bind(&DbClient::register_login_dbvisit_c, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::REGISTER_GAME_DBVISIT_C,
            std::bind(&DbClient::register_game_dbvisit_c, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::REGISTER_GATE_DBVISIT_C,
            std::bind(&DbClient::register_gate_dbvisit_c, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::LOADDATA_FROM_DBVISIT_C,
            std::bind(&DbClient::loaddata_from_dbvisit_c, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::LOADDATA_MORE_FROM_DBVISIT_C,
            std::bind(&DbClient::loaddata_more_from_dbvisit_c, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::INSERTDATA_TO_DBVISIT_C,
            std::bind(&DbClient::insertdata_to_dbvisit_c, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::UPDATA_TO_DBVISIT_C,
            std::bind(&DbClient::updata_to_dbvisit_c, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::DELETEDATA_TO_DBVISIT_C,
            std::bind(&DbClient::deletedata_to_dbvisit_c, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::LOGIN_CLIENT_GATE_C,
            std::bind(&DbClient::login_client_gate_c, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::CLIOFFLINE_GATE_ALL_C,
            std::bind(&DbClient::clioffline_gate_all_c, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::RECONNECT_CLIENT_GATE_C,
            std::bind(&DbClient::reconnect_client_gate_c, this, std::placeholders::_1, std::placeholders::_2) },
    };
}

DbClient::~DbClient()
{
    std::string type = frmpub::Basic::connectname(sif().st());
    std::string key = shynet::utils::stringop::str_format("%s_%d", type.c_str(), sif().sid());
    redis::Redis& redis = shynet::utils::Singleton<redis::Redis>::get_instance();
    try {
        redis.del(key);
    } catch (const std::exception& err) {
        THROW_EXCEPTION(err.what());
    }
    std::string str;
    if (active() == net::CloseType::SERVER_CLOSE) {
        str = "服务器dbvisit主动关闭连接";
    } else {
        str = frmpub::Basic::connectname(sif().st()) + std::string("客户端主动关闭连接");
    }
    LOG_INFO << str << "[ip:" << remote_addr()->ip() << ":" << remote_addr()->port() << "]";
}

int DbClient::default_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    //通知lua的onMessage函数
    shynet::utils::Singleton<lua::LuaEngine>::instance().append(
        std::make_shared<frmpub::OnMessageTask<DbClient>>(
            std::dynamic_pointer_cast<DbClient>(shared_from_this()), obj, enves));
    return 0;
}

void DbClient::close(net::CloseType active)
{
    frmpub::Client::close(active);
    shynet::utils::Singleton<DbClientMgr>::instance().remove(iobuf()->fd());
}

bool DbClient::verify_register(const protocc::ServerInfo& sif)
{
    redis::Redis& redis = shynet::utils::Singleton<redis::Redis>::get_instance();
    std::string type = frmpub::Basic::connectname(sif.st());
    std::string key = shynet::utils::stringop::str_format("%s_%d", type.c_str(), sif.sid());
    std::unordered_map<std::string, std::string> info;
    try {
        if (redis.exists(key) == 0) {
            redis.hmset(key,
                {
                    std::make_pair("ip", sif.ip()),
                    std::make_pair("port", std::to_string(sif.port())),
                    std::make_pair("sid", std::to_string(sif.sid())),
                    std::make_pair("type", type),
                    std::make_pair("name", sif.name()),
                });
            return true;
        } else {
            redis.hgetall(key, std::inserter(info, info.begin()));
            if (sif.ip() != info["ip"] || std::to_string(sif.port()) != info["port"]) {
                return false;
            }
            return true;
        }
    } catch (const std::exception& err) {
        THROW_EXCEPTION(err.what());
    }
    return false;
}

int DbClient::errcode(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::errcode err;
    if (err.ParseFromString(data->msgdata()) == true) {
        LOG_DEBUG << "错误码:" << err.code() << " 描述:" << err.desc();
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}

int DbClient::register_world_dbvisit_c(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::register_world_dbvisit_c msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {
        if (verify_register(msgc.sif()) == true) {
            LOG_DEBUG << frmpub::Basic::connectname(protocc::ServerType::WORLD) << "注册"
                      << " sid:" << msgc.sif().sid() << " ["
                      << msgc.sif().ip() << ":" << msgc.sif().port() << "]";
            set_sif(msgc.sif());
            protocc::register_world_dbvisit_s msgs;
            msgs.set_result(0);
            send_proto(protocc::REGISTER_WORLD_DBVISIT_S, &msgs);
        } else {
            protocc::register_world_dbvisit_s msgs;
            msgs.set_result(1);
            send_proto(protocc::REGISTER_WORLD_DBVISIT_S, &msgs);
            return -1;
        }
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}

int DbClient::register_login_dbvisit_c(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::register_login_dbvisit_c msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {
        if (verify_register(msgc.sif()) == true) {
            LOG_DEBUG << frmpub::Basic::connectname(protocc::ServerType::LOGIN) << "注册"
                      << " sid:" << msgc.sif().sid() << " ["
                      << msgc.sif().ip() << ":" << msgc.sif().port() << "]";
            set_sif(msgc.sif());
            protocc::register_login_dbvisit_s msgs;
            msgs.set_result(0);
            send_proto(protocc::REGISTER_LOGIN_DBVISIT_S, &msgs);
        } else {
            protocc::register_login_dbvisit_s msgs;
            msgs.set_result(1);
            send_proto(protocc::REGISTER_LOGIN_DBVISIT_S, &msgs);
            return -1;
        }
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}

int DbClient::register_game_dbvisit_c(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::register_game_dbvisit_c msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {
        if (verify_register(msgc.sif()) == true) {
            LOG_DEBUG << frmpub::Basic::connectname(protocc::ServerType::GAME) << "注册"
                      << " sid:" << msgc.sif().sid() << " ["
                      << msgc.sif().ip() << ":" << msgc.sif().port() << "]";
            set_sif(msgc.sif());
            protocc::register_game_dbvisit_s msgs;
            msgs.set_result(0);
            send_proto(protocc::REGISTER_GAME_DBVISIT_S, &msgs);
        } else {
            protocc::register_game_dbvisit_s msgs;
            msgs.set_result(1);
            send_proto(protocc::REGISTER_GAME_DBVISIT_S, &msgs);
            return -1;
        }

    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}

int DbClient::register_gate_dbvisit_c(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::register_gate_dbvisit_c msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {
        if (verify_register(msgc.sif()) == true) {
            LOG_DEBUG << frmpub::Basic::connectname(protocc::ServerType::GATE) << "注册"
                      << " sid:" << msgc.sif().sid() << " ["
                      << msgc.sif().ip() << ":" << msgc.sif().port() << "]";
            set_sif(msgc.sif());
            protocc::register_gate_dbvisit_s msgs;
            msgs.set_result(0);
            send_proto(protocc::REGISTER_GATE_DBVISIT_S, &msgs);
        } else {
            protocc::register_gate_dbvisit_s msgs;
            msgs.set_result(1);
            send_proto(protocc::REGISTER_GATE_DBVISIT_S, &msgs);
            return -1;
        }

    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}

int DbClient::loaddata_from_dbvisit_c(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::loaddata_from_dbvisit_c msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {
        try {
            protocc::loaddata_from_dbvisit_s msgs;
            std::unordered_map<std::string, std::string> data;
            for (int i = 0; i < msgc.fields_size(); i++) {
                data[msgc.fields(i).key()] = msgc.fields(i).value();
            }
            Datahelp::ErrorCode err = shynet::utils::Singleton<Datahelp>::instance().getdata(msgc.cache_key(), data,
                static_cast<Datahelp::OperType>(msgc.opertype()));
            if (err == Datahelp::ErrorCode::OK) {
                for (auto&& [key, value] : data) {
                    auto field = msgs.add_fields();
                    field->set_key(key);
                    field->set_value(value);
                }
            } else {
                msgs.set_result(1);
            }
            msgs.set_tag(msgc.tag());
            LOG_DEBUG << "LOADDATA_FROM_DBVISIT_S:" << msgs.SerializeAsString().length();
            send_proto(protocc::LOADDATA_FROM_DBVISIT_S, &msgs, enves.get());
            LOG_DEBUG << "发送消息" << frmpub::Basic::msgname(protocc::LOADDATA_FROM_DBVISIT_S) << "到"
                      << frmpub::Basic::connectname(sif().st())
                      << " result:" << msgs.result();
        } catch (const std::exception& err) {
            SEND_ERR(protocc::DB_CACHE_ERROR, err.what());
        }
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}

int DbClient::loaddata_more_from_dbvisit_c(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::loaddata_more_from_dbvisit_c msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {
        try {
            protocc::loaddata_more_from_dbvisit_s msgs;
            std::unordered_map<std::string, std::string> data;
            for (int i = 0; i < msgc.fields_size(); i++) {
                data[msgc.fields(i).key()] = msgc.fields(i).value();
            }

            moredataptr resdata = shynet::utils::Singleton<Datahelp>::instance().getdata_more(msgc.condition(), data,
                msgc.sort(), msgc.limit(), static_cast<Datahelp::OperType>(msgc.opertype()));
            for (auto& i : *resdata) {
                auto obj = msgs.add_objs();
                for (auto&& [key, value] : i) {
                    auto field = obj->add_fields();
                    field->set_key(key);
                    field->set_value(value);
                }
            }
            if (resdata->size() == 0) {
                msgs.set_result(1);
            }
            msgs.set_tag(msgc.tag());
            LOG_DEBUG << "LOADDATA_FROM_DBVISIT_S大小:" << msgs.SerializeAsString().length();
            send_proto(protocc::LOADDATA_MORE_FROM_DBVISIT_S, &msgs, enves.get());
            LOG_DEBUG << "发送消息" << frmpub::Basic::msgname(protocc::LOADDATA_MORE_FROM_DBVISIT_S) << "到"
                      << frmpub::Basic::connectname(sif().st())
                      << " result:" << msgs.result();
        } catch (const std::exception& err) {
            SEND_ERR(protocc::DB_CACHE_ERROR, err.what());
        }
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}

int DbClient::insertdata_to_dbvisit_c(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::insertdata_to_dbvisit_c msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {
        try {
            protocc::insertdata_to_dbvisit_s msgs;
            std::unordered_map<std::string, std::string> data;
            for (int i = 0; i < msgc.fields_size(); i++) {
                data[msgc.fields(i).key()] = msgc.fields(i).value();
            }
            shynet::utils::Singleton<Datahelp>::instance().insertdata(msgc.cache_key(), data,
                static_cast<Datahelp::OperType>(msgc.opertype()));

            msgs.set_tag(msgc.tag());
            send_proto(protocc::INSERTDATA_TO_DBVISIT_S, &msgs, enves.get());
            LOG_DEBUG << "发送消息" << frmpub::Basic::msgname(protocc::INSERTDATA_TO_DBVISIT_S) << "到"
                      << frmpub::Basic::connectname(sif().st())
                      << " result:" << msgs.result();
        } catch (const std::exception& err) {
            SEND_ERR(protocc::DB_CACHE_ERROR, err.what());
        }
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}

int DbClient::updata_to_dbvisit_c(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::updata_to_dbvisit_c msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {
        try {
            protocc::updata_to_dbvisit_s msgs;
            std::unordered_map<std::string, std::string> data;
            for (int i = 0; i < msgc.fields_size(); i++) {
                data[msgc.fields(i).key()] = msgc.fields(i).value();
            }
            shynet::utils::Singleton<Datahelp>::instance().updata(msgc.cache_key(), data,
                static_cast<Datahelp::OperType>(msgc.opertype()));
            msgs.set_tag(msgc.tag());
            send_proto(protocc::UPDATA_TO_DBVISIT_S, &msgs, enves.get());
            LOG_DEBUG << "发送消息" << frmpub::Basic::msgname(protocc::UPDATA_TO_DBVISIT_S) << "到"
                      << frmpub::Basic::connectname(sif().st())
                      << " result:" << msgs.result();
        } catch (const std::exception& err) {
            SEND_ERR(protocc::DB_CACHE_ERROR, err.what());
        }
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}

int DbClient::deletedata_to_dbvisit_c(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::deletedata_to_dbvisit_c msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {
        try {
            protocc::deletedata_to_dbvisit_s msgs;
            shynet::utils::Singleton<Datahelp>::instance().deletedata(msgc.cache_key(),
                static_cast<Datahelp::OperType>(msgc.opertype()));
            msgs.set_tag(msgc.tag());
            send_proto(protocc::DELETEDATA_TO_DBVISIT_S, &msgs, enves.get());
            LOG_DEBUG << "发送消息" << frmpub::Basic::msgname(protocc::DELETEDATA_TO_DBVISIT_S) << "到"
                      << frmpub::Basic::connectname(sif().st())
                      << " result:" << msgs.result();
        } catch (const std::exception& err) {
            SEND_ERR(protocc::DB_CACHE_ERROR, err.what());
        }
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}

int DbClient::login_client_gate_c(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::login_client_gate_c msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {
        try {
            //取出路由信息
            std::stack<FilterData::Envelope> routing = *enves;
            FilterData::Envelope gate_enve = routing.top();
            net::IPAddress gate_addr(&gate_enve.addr);
            routing.pop();
            FilterData::Envelope cli_enve = routing.top();
            net::IPAddress cli_addr(&cli_enve.addr);
            routing.pop();

            int result = 0; //默认失败
            //缓存过期24小时
            std::string accountid = "0"; //账号id
            std::string roleid = "0"; //角色id
            auto temp = shynet::utils::stringop::split(data->extend(), ",");
            if (temp.size() != 3) {
                std::stringstream stream;
                stream << "附加信息解析错误 extend:" << data->extend();
                SEND_ERR(protocc::EXTEND_FORMAT_ERR, stream.str());
                return 0;
            }
            std::string gate_sid = temp[0];
            std::string login_sid = temp[1];
            std::string game_sid = temp[2];
            //旧的网关服id
            std::string old_gate_sid = "0";

            //账号数据
            std::unordered_map<std::string, std::string> user_data {
                { "_id", accountid },
                { "roleid", roleid },
                { "clientaddr", cli_addr.ip() },
                { "clientport", std::to_string(cli_addr.port()) },
                { "gate_sid", gate_sid },
                { "login_sid", login_sid },
                { "game_sid", game_sid },
                { "online", "1" },
                { "platform_key", msgc.platform_key() },
            };
            redis::Redis& redis = shynet::utils::Singleton<redis::Redis>::get_instance();
            //通过平台key取出账号cache_key_value
            redis::OptionalString cache_key_value = redis.get(msgc.platform_key());
            Datahelp& help = shynet::utils::Singleton<Datahelp>::instance();

            if (!cache_key_value) {
                ///缓存没有玩家数据,查询数据库
                std::string tablename = "account";
                std::string sql = shynet::utils::stringop::str_format("platform_key='%s'", msgc.platform_key().c_str());
                Datahelp::ErrorCode code = help.getdata_from_db(tablename, "", user_data, sql);

                if (code == Datahelp::ErrorCode::NOT_DATA) {
                    //没有玩家数据，注册玩家
                    accountid = std::to_string(shynet::utils::Singleton<shynet::utils::IdWorker>::get_instance().getid());
                    user_data["_id"] = accountid;
                    help.insert_db(tablename, accountid, user_data);
                    LOG_DEBUG << "注册玩家 accountid:" << accountid << " roleid:" << roleid;
                } else {
                    //取出玩家数据
                    accountid = user_data["_id"];
                    roleid = user_data["roleid"];
                    old_gate_sid = user_data["gate_sid"];
                }
                std::string key = std::string("account_") + accountid;
                //玩家平台key缓存X小时
                redis.set(msgc.platform_key(), key, oneday_);
                //缓存玩家数据
                help.updata_cache(key, user_data);
                LOG_DEBUG << "从db取出账号信息 accountid:" << accountid << " roleid:" << roleid;
            } else {
                //玩家平台key缓存X小时
                redis.expire(msgc.platform_key(), oneday_);
                //通过cache_key从缓存取出玩家数据
                Datahelp::ErrorCode error = help.getdata_from_cache(*cache_key_value, user_data);
                if (error == Datahelp::ErrorCode::OK) {
                    accountid = user_data["_id"];
                    roleid = user_data["roleid"];
                    old_gate_sid = user_data["gate_sid"];

                    //更新关联数据
                    std::unordered_map<std::string, std::string> data = {
                        { "clientaddr", cli_addr.ip() },
                        { "clientport", std::to_string(cli_addr.port()) },
                        { "gate_sid", gate_sid },
                        { "login_sid", login_sid },
                        { "game_sid", game_sid },
                        { "online", "1" },
                        { "platform_key", msgc.platform_key() },
                    };
                    help.updata(*cache_key_value, data, Datahelp::OperType::ALL);
                    LOG_DEBUG << "从cache取出账号信息 accountid:" << accountid << " roleid:" << roleid;
                } else {
                    LOG_WARN << "缓存中没有账号数据";
                    redis.del(msgc.platform_key());
                    result = 1;
                }
            }

            if (old_gate_sid != "") {
                if (old_gate_sid != gate_sid) {
                    //跨服顶号处理
                    protocc::repeatlogin_client_gate_s notify;
                    notify.set_aid(accountid);
                    send_proto(protocc::REPEATLOGIN_CLIENT_GATE_G, &notify, nullptr, &old_gate_sid);
                    LOG_DEBUG << "发送消息" << frmpub::Basic::msgname(protocc::REPEATLOGIN_CLIENT_GATE_G) << "到"
                              << frmpub::Basic::connectname(sif().st());
                }
            }
            if (result == 0) {
                if (cache_key_value) {
                    //如果有则清除断线重连有效时间
                    std::string key = *cache_key_value + std::string("_disconnect");
                    redis.del(key);
                }
            }
            //assert(accountid.empty() == false);
            //发送登录结果
            protocc::login_client_gate_s msgs;
            msgs.set_result(result);
            msgs.set_aid(accountid);
            msgs.set_roleid(strtoull(roleid.c_str(), nullptr, 0));
            send_proto(protocc::LOGIN_CLIENT_GATE_S, &msgs, enves.get(), &game_sid);
            LOG_DEBUG << "发送消息" << frmpub::Basic::msgname(protocc::LOGIN_CLIENT_GATE_S) << "到"
                      << frmpub::Basic::connectname(sif().st())
                      << " result:" << msgs.result()
                      << " aid:" << msgs.aid();
        } catch (const std::exception& err) {
            SEND_ERR(protocc::DB_CACHE_ERROR, err.what());
        }
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}

int DbClient::clioffline_gate_all_c(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::clioffline_gate_all_c msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {
        try {
            LOG_DEBUG << "玩家下线 账号id:" << msgc.aid();
            std::string key = std::string("account_") + msgc.aid();
            std::unordered_map<std::string, std::string> fields {
                { "clientaddr", "" },
                { "clientport", "" },
            };

            Datahelp& help = shynet::utils::Singleton<Datahelp>::instance();
            Datahelp::ErrorCode error = help.getdata(key, fields, Datahelp::OperType::ALL, false);
            if (error == Datahelp::ErrorCode::OK) {
                if (msgc.ip() == fields["clientaddr"]
                    && std::to_string(msgc.port()) == fields["clientport"]) {

                    //清除下线玩家登陆时关联的数据
                    std::unordered_map<std::string, std::string> data = {
                        { "clientaddr", "" },
                        { "clientport", "" },
                        { "online", "0" },
                        { "login_sid", "" },
                        { "game_sid", "" },
                        { "gate_sid", "" },
                    };
                    help.updata(key, data, Datahelp::OperType::ALL);

                    redis::Redis& redis = shynet::utils::Singleton<redis::Redis>::get_instance();
                    //设置断线重连有效时间30秒
                    redis.set(key + std::string("_disconnect"), "0", std::chrono::seconds(30));
                }
            } else {
                LOG_WARN << "账号不存在 key=" << key << " 失败";
            }
        } catch (const std::exception& err) {
            SEND_ERR(protocc::DB_CACHE_ERROR, err.what());
        }
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}

int DbClient::reconnect_client_gate_c(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::reconnect_client_gate_c msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {
        protocc::reconnect_client_gate_s msgs;
        try {
            redis::Redis& redis = shynet::utils::Singleton<redis::Redis>::get_instance();
            std::string key = std::string("account_") + msgc.aid() + std::string("_disconnect");
            //验证断线重连是否有效
            if (redis.exists(key)) {
                std::stack<FilterData::Envelope> routing = *enves;
                FilterData::Envelope gate_enve = routing.top();
                net::IPAddress gate_addr(&gate_enve.addr);
                routing.pop();
                FilterData::Envelope cli_enve = routing.top();
                net::IPAddress cli_addr(&cli_enve.addr);
                routing.pop();

                //更新断线重连后关联的数据
                std::unordered_map<std::string, std::string> clidata = {
                    { "clientaddr", cli_addr.ip() },
                    { "clientport", std::to_string(cli_addr.port()) },
                    { "online", "1" },
                    { "login_sid", std::to_string(msgc.loginid()) },
                    { "game_sid", std::to_string(msgc.gameid()) },
                    { "gate_sid", data->extend() },
                };
                shynet::utils::Singleton<Datahelp>::instance().updata(std::string("account_") + msgc.aid(), clidata,
                    Datahelp::OperType::ALL);
                redis.del(key);
            } else {
                msgs.set_result(1);
            }
            msgs.set_aid(msgc.aid());
            msgs.set_loginid(msgc.loginid());
            msgs.set_gameid(msgc.gameid());
            send_proto(protocc::RECONNECT_CLIENT_GATE_S, &msgs, enves.get());
            LOG_DEBUG << "发送消息" << frmpub::Basic::msgname(protocc::RECONNECT_CLIENT_GATE_S) << "到"
                      << frmpub::Basic::connectname(sif().st())
                      << " result:" << msgs.result();
        } catch (const std::exception& err) {
            SEND_ERR(protocc::DB_CACHE_ERROR, err.what());
        }
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}

}
