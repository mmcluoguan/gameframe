#include "game/gameclient.h"
#include "3rd/fmt/format.h"
#include "frmpub/luacallbacktask.h"
#include "game/gameclientmgr.h"
#include "shynet/lua/luaengine.h"
#include "shynet/utils/elapsed.h"

namespace game {
GameClient::GameClient(std::shared_ptr<net::IPAddress> remote_addr,
    std::shared_ptr<net::IPAddress> listen_addr,
    std::shared_ptr<events::EventBuffer> iobuf)
    : frmpub::Client(remote_addr, listen_addr, iobuf)
{
    LOG_INFO << "新客户端连接 [ip:" << remote_addr->ip() << ":" << remote_addr->port() << "]";

    pmb_ = {
        { protocc::ERRCODE,
            std::bind(&GameClient::errcode, this, std::placeholders::_1, std::placeholders::_2) },
        { protocc::REGISTER_GATE_GAME_C,
            std::bind(&GameClient::register_gate_game_c, this, std::placeholders::_1, std::placeholders::_2) },
    };
}

GameClient::~GameClient()
{
    std::string str;
    if (active() == net::CloseType::SERVER_CLOSE) {
        str = "服务器game主动关闭连接";
    } else {
        str = frmpub::Basic::connectname(sif().st()) + "客户端主动关闭连接";
    }
    LOG_INFO << str << "[ip:" << remote_addr()->ip() << ":" << remote_addr()->port() << "]";
}

int GameClient::input_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    auto cb = [&]() {
        auto it = pmb_.find(obj->msgid());
        if (it != pmb_.end()) {
            return it->second(obj, enves);
        } else {
            //通知lua的onMessage函数
            shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
                std::make_shared<frmpub::OnMessageTask<GameClient>>(shared_from_this(), obj, enves));
        }
        return 0;
    };

#ifdef USE_DEBUG
    std::string str = fmt::format("工作线程单任务执行 {}", frmpub::Basic::msgname(obj->msgid()));
    shynet::utils::elapsed(str.c_str());
    return cb();
#else
    return cb();
#endif
}

void GameClient::close(net::CloseType active)
{
    frmpub::Client::close(active);
    shynet::utils::Singleton<GameClientMgr>::instance().remove(iobuf()->fd());
}

int GameClient::errcode(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
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

int GameClient::register_gate_game_c(std::shared_ptr<protocc::CommonObject> data,
    std::shared_ptr<std::stack<FilterData::Envelope>> enves)
{
    protocc::register_gate_game_c msgc;
    if (msgc.ParseFromString(data->msgdata()) == true) {
        LOG_DEBUG << frmpub::Basic::connectname(protocc::ServerType::GATE) << "注册"
                  << " sid:" << msgc.sif().sid() << " ["
                  << msgc.sif().ip() << ":" << msgc.sif().port() << "]";
        set_sif(msgc.sif());
        protocc::register_gate_game_s msgs;
        msgs.set_result(0);
        send_proto(protocc::REGISTER_GATE_GAME_S, &msgs);
    } else {
        std::stringstream stream;
        stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
        SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
    }
    return 0;
}
}
