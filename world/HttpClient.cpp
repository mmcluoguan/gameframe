#include "world/httpclient.h"
#include "frmpub/luacallbacktask.h"
#include "frmpub/protocc/game.pb.h"
#include "frmpub/protocc/gate.pb.h"
#include "frmpub/protocc/login.pb.h"
#include "frmpub/protocc/world.pb.h"
#include "shynet/lua/luaengine.h"
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
    };
}

HttpClient::~HttpClient()
{
    std::string str;
    if (active()) {
        str = "服务器http主动关闭连接";
    } else {
        str = "客户端主动关闭连接";
    }
    LOG_INFO << str << "[ip:" << remote_addr()->ip() << ":" << remote_addr()->port() << "]";
}

int HttpClient::input_handle(std::shared_ptr<rapidjson::Document> doc, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    if (doc != nullptr) {
        int msgid = (*doc)["msgid"].GetInt();
        auto it = jmb_.find(msgid);
        if (it != jmb_.end()) {
            return it->second(doc, enves);
        } else {
            //通知lua的onMessage函数
            shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
                std::make_shared<frmpub::OnMessageTask<HttpClient>>(shared_from_this(), doc, enves));
        }
    }
    return 0;
}

void HttpClient::close(bool active)
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
    rapidjson::Value msgs;
    msgs.SetObject();
    msgs.AddMember("games", rapidjson::Value(rapidjson::kArrayType), doc->GetAllocator());
    auto clis = shynet::utils::Singleton<WorldClientMgr>::instance().clis();
    for (auto&& [key, cli] : clis) {
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
    }
    send_json(int(frmpub::JosnMsgId::GETGAMELIST_ADMIN_WORLD_S), &msgs, enves.get());
    return 0;
}

int HttpClient::noticeserver_admin_world_c(std::shared_ptr<rapidjson::Document> doc, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    try {
        rapidjson::Value& msgdata = frmpub::get_json_value(*doc, "msgdata");
        int32_t sid = frmpub::get_json_value(msgdata, "sid").GetInt();
        std::string info = frmpub::get_json_value(msgdata, "info").GetString();
        auto games = shynet::utils::Singleton<WorldClientMgr>::instance().clis();
        bool flag = false;
        for (const auto& gs : games) {
            auto sif = gs.second->sif();
            if (sid != 0 && sid != sif.sid()) {
                continue;
            }
            if (sif.st() == protocc::ServerType::GAME) {
                flag = true;
                LOG_DEBUG << "通知区服服务器 name:" << sif.name() << " sid:" << sif.sid();
                protocc::noticeserver_world_game_g gmsg;
                gmsg.set_info(info);
                gs.second->send_proto(protocc::NOTICESERVER_WORLD_GAME_G, &gmsg, enves.get());
            }
        }
        rapidjson::Value result(rapidjson::kObjectType);
        result.AddMember("result", flag ? 0 : 1, doc->GetAllocator());
        send_json(int(frmpub::JosnMsgId::NOTICESERVER_ADMIN_WORLD_S), &result, enves.get());
    } catch (const std::exception& err) {
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, err.what());
    }
    return 0;
}

}
