#include "world/worldclient.h"
#include "3rd/fmt/format.h"
#include "frmpub/luacallbacktask.h"
#include "shynet/lua/luaengine.h"
#include "shynet/utils/elapsed.h"
#include "world/worldclientmgr.h"

namespace world {
WorldClient::WorldClient(std::shared_ptr<net::IPAddress> remote_addr,
    std::shared_ptr<net::IPAddress> listen_addr,
    std::shared_ptr<events::EventBuffer> iobuf)
    : frmpub::Client(remote_addr, listen_addr, iobuf)
{
    LOG_INFO << "新客户端连接 [ip:" << remote_addr->ip() << ":" << remote_addr->port() << "]";

    pmb_ = {
        { protocc::ERRCODE,
            std::bind(&WorldClient::errcode, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::REGISTER_LOGIN_WORLD_C,
            std::bind(&WorldClient::register_login_world_c, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::REGISTER_GAME_WORLD_C,
            std::bind(&WorldClient::register_game_world_c, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::REGISTER_GATE_WORLD_C,
            std::bind(&WorldClient::register_gate_world_c, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::CLIOFFLINE_GATE_ALL_C,
            std::bind(&WorldClient::clioffline_gate_all_c, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::LOGIN_CLIENT_GATE_S,
            std::bind(&WorldClient::login_client_gate_s, this, std::placeholders::_1, std::placeholders::_2) },
    };
}

WorldClient::~WorldClient()
{
    std::string str;
    if (active() == net::CloseType::SERVER_CLOSE) {
        str = "服务器world主动关闭连接";
    } else {
        str = frmpub::Basic::connectname(sif().st()) + "客户端主动关闭连接";
    }
    LOG_INFO << str << "[ip:" << remote_addr()->ip() << ":" << remote_addr()->port() << "]";
}

int WorldClient::input_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    auto cb = [&]() {
        auto it = pmb_.find(obj->msgid());
        if (it != pmb_.end()) {
            return it->second(obj, enves);
        } else {
            //通知lua的onMessage函数
            shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
                std::make_shared<frmpub::OnMessageTask<WorldClient>>(shared_from_this(), obj, enves));
        }
        return 0;
    };
#ifdef USE_DEBUG
    std::string str = fmt::format("工作线程单任务执行 {}", frmpub::Basic::msgname(obj->msgid()));
    shynet::utils::elapsed(str.c_str());
    return cb();
#elif
    return cb();
#endif
}

void WorldClient::close(net::CloseType active)
{
    frmpub::Client::close(active);
    shynet::utils::Singleton<WorldClientMgr>::instance().remove(iobuf()->fd());
}

int WorldClient::errcode(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
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

int WorldClient::register_login_world_c(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::register_login_world_c msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {
        LOG_DEBUG << frmpub::Basic::connectname(protocc::ServerType::LOGIN) << "注册"
                  << " sid:" << msgc.sif().sid() << " ["
                  << msgc.sif().ip() << ":" << msgc.sif().port() << "]";
        set_sif(msgc.sif());
        protocc::register_login_world_s msgs;
        msgs.set_result(0);
        send_proto(protocc::REGISTER_LOGIN_WORLD_S, &msgs);

        //通知网关服务器登录服务器上线
        protocc::seronline_world_gate_g msgg;
        protocc::ServerInfo* sif = msgg.mutable_sif();
        *sif = this->sif();

        shynet::utils::Singleton<WorldClientMgr>::instance()
            .foreach_clis([&](int key, std::shared_ptr<WorldClient> cli) {
                if (cli->sif().st() == protocc::ServerType::GATE) {
                    cli->send_proto(protocc::SERONLINE_WORLD_GATE_G, &msgg);
                }
            });
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}
int WorldClient::register_game_world_c(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::register_game_world_c msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {
        LOG_DEBUG << frmpub::Basic::connectname(protocc::ServerType::GAME) << "注册"
                  << " sid:" << msgc.sif().sid() << " ["
                  << msgc.sif().ip() << ":" << msgc.sif().port() << "]";
        set_sif(msgc.sif());
        protocc::register_game_world_s msgs;
        msgs.set_result(0);
        send_proto(protocc::REGISTER_GAME_WORLD_S, &msgs);

        //通知网关服务器游戏服务器上线
        protocc::seronline_world_gate_g msgg;
        protocc::ServerInfo* sif = msgg.mutable_sif();
        *sif = this->sif();

        shynet::utils::Singleton<WorldClientMgr>::instance()
            .foreach_clis([&](int key, std::shared_ptr<WorldClient> cli) {
                if (cli->sif().st() == protocc::ServerType::GATE) {
                    cli->send_proto(protocc::SERONLINE_WORLD_GATE_G, &msgg);
                }
            });
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}
int WorldClient::register_gate_world_c(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::register_gate_world_c msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {
        LOG_DEBUG << frmpub::Basic::connectname(protocc::ServerType::GATE) << "注册"
                  << " sid:" << msgc.sif().sid() << " ["
                  << msgc.sif().ip() << ":" << msgc.sif().port() << "]";
        set_sif(msgc.sif());

        //返回已经在世界服注册的游戏和登录服务器信息
        protocc::register_gate_world_s msgs;
        msgs.set_result(0);
        shynet::utils::Singleton<WorldClientMgr>::instance()
            .foreach_clis([&](int key, std::shared_ptr<WorldClient> cli) {
                if (cli->sif().st() == protocc::ServerType::GAME
                    || cli->sif().st() == protocc::ServerType::LOGIN) {
                    protocc::ServerInfo* sif = msgs.add_sifs();
                    *sif = cli->sif();
                }
            });
        send_proto(protocc::REGISTER_GATE_WORLD_S, &msgs);
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}
int WorldClient::clioffline_gate_all_c(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::clioffline_gate_all_c msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {

    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}
int WorldClient::login_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    if (data->extend().empty() == false && data->extend() == "0") {
        auto game = shynet::utils::Singleton<WorldClientMgr>::instance().select_game();
        if (game != nullptr) {
            data->set_extend(std::to_string(game->sif().sid()));
            send_proto(data.get(), enves.get());
        } else {
            std::stringstream stream;
            stream << "没有可用的" << frmpub::Basic::connectname(protocc::ServerType::GAME) << "连接";
            SEND_ERR(protocc::GAME_NOT_EXIST, stream.str());
        }
    } else {
        std::stringstream stream;
        stream << "附加信息解析错误 extend:" << data->extend();
        SEND_ERR(protocc::EXTEND_FORMAT_ERR, stream.str());
    }
    return 0;
}
}
