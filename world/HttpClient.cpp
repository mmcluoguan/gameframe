#include "world/httpclient.h"
#include "frmpub/luacallbacktask.h"
#include "frmpub/protocc/game.pb.h"
#include "frmpub/protocc/gate.pb.h"
#include "frmpub/protocc/login.pb.h"
#include "frmpub/protocc/world.pb.h"
#include "shynet/lua/luaengine.h"
#include "world/httpclientmgr.h"

namespace world {
HttpClient::HttpClient(std::shared_ptr<net::IPAddress> remote_addr,
    std::shared_ptr<net::IPAddress> listen_addr,
    std::shared_ptr<events::EventBuffer> iobuf)
    : frmpub::Client(remote_addr, listen_addr, iobuf, true, 5L,
        shynet::protocol::FilterProces::ProtoType::HTTP, FilterData::ProtoData::JSON)
{
    LOG_INFO << "http新客户端连接 [ip:" << remote_addr->ip() << ":" << remote_addr->port() << "]";

    jmb_ = {

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

}
