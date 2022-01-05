#include "world/httpclient.h"
#include "3rd/fmt/format.h"
#include "frmpub/luacallbacktask.h"
#include "frmpub/protocc/game.pb.h"
#include "frmpub/protocc/gate.pb.h"
#include "frmpub/protocc/login.pb.h"
#include "frmpub/protocc/world.pb.h"
#include "shynet/lua/luaengine.h"
#include "shynet/utils/elapsed.h"
#include "shynet/utils/idworker.h"
#include "world/httpclientmgr.h"
#include "world/worldclientmgr.h"

namespace world {
HttpClient::HttpClient(std::shared_ptr<net::IPAddress> remote_addr,
    std::shared_ptr<net::IPAddress> listen_addr,
    std::shared_ptr<events::EventBuffer> iobuf)
    : frmpub::Client(remote_addr, listen_addr, iobuf, false, 5L,
        shynet::protocol::FilterProces::ProtoType::HTTP, FilterData::ProtoData::JSON)
{
    LOG_INFO << "http新客户端连接 [ip:" << remote_addr->ip() << ":" << remote_addr->port() << "]";

    jmb_ = {
        { int(protocc::ERRCODE),
            std::bind(&HttpClient::errcode, this, std::placeholders::_1, std::placeholders::_2) },
        { int(frmpub::JosnMsgId::GETGAMELIST_ADMIN_WORLD_C),
            std::bind(&HttpClient::getgamelist_admin_world_c, this, std::placeholders::_1, std::placeholders::_2) },
        { int(frmpub::JosnMsgId::NOTICESERVER_ADMIN_WORLD_C),
            std::bind(&HttpClient::noticeserver_admin_world_c, this, std::placeholders::_1, std::placeholders::_2) },
        { int(frmpub::JosnMsgId::SYSEMAIL_ADMIN_WORLD_C),
            std::bind(&HttpClient::sysemail_admin_world_c, this, std::placeholders::_1, std::placeholders::_2) },
    };
}

HttpClient::~HttpClient()
{
    std::string str;
    if (active() == net::CloseType::SERVER_CLOSE) {
        str = "服务器http主动关闭连接";
    } else {
        str = "客户端主动关闭连接";
    }
    LOG_INFO << str << "[ip:" << remote_addr()->ip() << ":" << remote_addr()->port() << "]";
}

int HttpClient::input_handle(std::shared_ptr<rapidjson::Document> doc, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    int msgid = (*doc)["msgid"].GetInt();
    auto cb = [&]() {
        auto it = jmb_.find(msgid);
        if (it != jmb_.end()) {
            return it->second(doc, enves);
        } else {
            //通知lua的onMessage函数
            shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
                std::make_shared<frmpub::OnMessageTask<HttpClient>>(shared_from_this(), doc, enves));
        }
        return 0;
    };
#ifdef USE_DEBUG
    std::string str = fmt::format("工作线程单任务执行 {}", frmpub::Basic::msgname(msgid));
    shynet::utils::elapsed(str.c_str());
    return cb();
#elif
    return cb();
#endif
}

void HttpClient::close(net::CloseType active)
{
    frmpub::Client::close(active);
    shynet::utils::Singleton<HttpClientMgr>::instance().remove(iobuf()->fd());
}

int HttpClient::errcode(std::shared_ptr<rapidjson::Document> doc, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    try {
        rapidjson::Value& msgdata = frmpub::get_json_value(*doc, "msgdata");
        int32_t code = frmpub::get_json_value(msgdata, "code").GetInt();
        const char* desc = frmpub::get_json_value(msgdata, "desc").GetString();
        LOG_DEBUG << "错误码:" << code << " 描述:" << desc;
    } catch (const std::exception& err) {
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, err.what());
    }
    return 0;
}

int HttpClient::getgamelist_admin_world_c(std::shared_ptr<rapidjson::Document> doc, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    try {
        rapidjson::Value msgs;
        msgs.SetObject();
        msgs.AddMember("games", rapidjson::Value(rapidjson::kArrayType), doc->GetAllocator());
        shynet::utils::Singleton<WorldClientMgr>::instance()
            .foreach_clis([&](int key, std::shared_ptr<WorldClient> cli) {
                if (cli->sif().st() == protocc::ServerType::GAME) {
                    rapidjson::Value game_json;
                    game_json.SetObject();
                    game_json.AddMember("ip", rapidjson::StringRef(cli->sif().ip().c_str()), doc->GetAllocator());
                    game_json.AddMember("port", cli->sif().port(), doc->GetAllocator());
                    game_json.AddMember("st", cli->sif().st(), doc->GetAllocator());
                    game_json.AddMember("sid", cli->sif().sid(), doc->GetAllocator());
                    game_json.AddMember("name", rapidjson::StringRef(cli->sif().name().c_str()), doc->GetAllocator());
                    msgs["games"].PushBack(game_json, doc->GetAllocator());
                }
            });
        send_json(frmpub::JosnMsgId::GETGAMELIST_ADMIN_WORLD_S, &msgs, enves.get());
    } catch (const std::exception& err) {
        int msgid = (*doc)["msgid"].GetInt();
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(msgid) << "解析错误 " << err.what();
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}

int HttpClient::noticeserver_admin_world_c(std::shared_ptr<rapidjson::Document> doc, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    try {
        rapidjson::Value& msgdata = frmpub::get_json_value(*doc, "msgdata");
        int32_t sid = frmpub::get_json_value(msgdata, "sid").GetInt();
        std::string info = frmpub::get_json_value(msgdata, "info").GetString();
        bool flag = false;
        shynet::utils::Singleton<WorldClientMgr>::instance()
            .foreach_clis([&](int key, std::shared_ptr<WorldClient> cli) {
                auto sif = cli->sif();
                if (sid != 0 && sid != sif.sid()) {
                    return;
                }
                if (sif.st() == protocc::ServerType::GAME) {
                    flag = true;
                    LOG_DEBUG << "通知区服服务器 name:" << sif.name() << " sid:" << sif.sid();
                    protocc::noticeserver_world_game_g gmsg;
                    gmsg.set_info(info);
                    cli->send_proto(protocc::NOTICESERVER_WORLD_GAME_G, &gmsg, enves.get());
                }
            });
        rapidjson::Value result(rapidjson::kObjectType);
        result.AddMember("result", flag ? 0 : 1, doc->GetAllocator());
        send_json(frmpub::JosnMsgId::NOTICESERVER_ADMIN_WORLD_S, &result, enves.get());
    } catch (const std::exception& err) {
        int msgid = (*doc)["msgid"].GetInt();
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(msgid) << "解析错误 " << err.what();
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}

int HttpClient::sysemail_admin_world_c(std::shared_ptr<rapidjson::Document> doc, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    try {
        rapidjson::Value msgs(rapidjson::kObjectType);
        int result = 0;
        rapidjson::Value& msgdata = frmpub::get_json_value(*doc, "msgdata");
        int32_t type = frmpub::get_json_value(msgdata, "type").GetInt();
        if (type != 1) {
            result = 1; //邮件类型错误
        } else {
            int32_t sid = frmpub::get_json_value(msgdata, "sid").GetInt();
            std::string title = frmpub::get_json_value(msgdata, "title").GetString();
            std::string info = frmpub::get_json_value(msgdata, "info").GetString();
            int32_t rid = frmpub::get_json_value(msgdata, "rid").GetInt();
            int32_t time = frmpub::get_json_value(msgdata, "time").GetInt();
            bool flag = false;
            shynet::utils::Singleton<WorldClientMgr>::instance()
                .foreach_clis([&](int key, std::shared_ptr<WorldClient> cli) {
                    auto sif = cli->sif();
                    if (sif.st() == protocc::ServerType::GAME) {
                        if (sid == -1 || sif.sid() == sid) {
                            flag = true;
                            //转发邮件到区服
                            protocc::sysemail_world_game_g gmsg;
                            uint64_t mailid = shynet::utils::Singleton<shynet::utils::IdWorker>::get_instance().getid();
                            gmsg.set_id(mailid);
                            gmsg.set_title(title);
                            gmsg.set_info(info);
                            gmsg.set_type(type);
                            gmsg.set_rid(rid);
                            gmsg.set_time(time);
                            try {
                                auto annex = frmpub::get_json_value(msgdata, "annex").GetObject();
                                auto gmsg_annex = gmsg.mutable_annex();
                                gmsg_annex->set_gold(frmpub::get_json_value(annex, "gold").GetInt());
                                gmsg_annex->set_diamond(frmpub::get_json_value(annex, "diamond").GetInt());
                                try {
                                    auto goods = frmpub::get_json_value(annex, "goods").GetArray();
                                    for (auto& it : goods) {
                                        auto cfgid = frmpub::get_json_value(it, "cfgid").GetInt();
                                        auto num = frmpub::get_json_value(it, "num").GetInt();

                                        auto gmsg_annex_goods = gmsg_annex->add_goods();
                                        gmsg_annex_goods->set_cfgid(cfgid);
                                        gmsg_annex_goods->set_num(num);
                                    }
                                } catch (const std::exception& err) {
                                }
                            } catch (const std::exception& err) {
                            }
                            cli->send_proto(protocc::SYSEMAIL_WORLD_GAME_G, &gmsg, enves.get());
                            LOG_DEBUG << "转发邮件到区服 name:" << sif.name() << " sid:" << sif.sid();
                        }
                    }
                });
            if (flag == false) {
                result = 2; //区服不存在
            }
        }
        msgs.AddMember("result", result, doc->GetAllocator());
        send_json(frmpub::JosnMsgId::SYSEMAIL_ADMIN_WORLD_S, &msgs, enves.get());
    } catch (const std::exception& err) {
        int msgid = (*doc)["msgid"].GetInt();
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(msgid) << "解析错误 " << err.what();
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }

    return 0;
}

}
