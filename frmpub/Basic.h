#ifndef FRMPUB_BASIC_H
#define FRMPUB_BASIC_H

#include "frmpub/protocc/client.pb.h"
#include "frmpub/protocc/common.pb.h"
#include "frmpub/protocc/dbvisit.pb.h"
#include "frmpub/protocc/game.pb.h"
#include "frmpub/protocc/gate.pb.h"
#include "frmpub/protocc/internal.pb.h"
#include "frmpub/protocc/login.pb.h"
#include "frmpub/protocc/world.pb.h"
#include "shynet/events/eventbuffer.h"
#include <curl/curl.h>
#include <rapidjson/document.h>
#include <stack>
#include <unordered_map>
namespace protocc = frmpub::protocc;

#ifndef SEND_ERR
#define SEND_ERR(code, str) \
    LOG_DEBUG << (str);     \
    send_errcode((code), (str));
#define SEND_ERR_EX(code, str, enves) \
    LOG_DEBUG << (str);               \
    send_errcode((code), (str), (enves));
#endif // !SEND_ERR

namespace frmpub {

enum class JosnMsgId {
    ADMIN_WORLD_BEGIN = 5001,
    GETGAMELIST_ADMIN_WORLD_C, //获取区服信息
    GETGAMELIST_ADMIN_WORLD_S,
    NOTICESERVER_ADMIN_WORLD_C, //广播通知消息
    NOTICESERVER_ADMIN_WORLD_S,

    ADMIN_WORLD_END = 6000,
};

rapidjson::Value& get_json_value(rapidjson::Value& jv, std::string key);
rapidjson::Value& get_json_value(rapidjson::Document& doc, std::string key);

class Basic {
public:
    static std::string msgname(int);
    static std::string connectname(protocc::ServerType st);

private:
    static std::string internal_msgname(int);
    static std::string client_msgname(int);
};

class Sms {
public:
    static std::string send(std::string url);

private:
    static size_t req_reply(void* ptr, size_t size, size_t nmemb, void* stream);
};
}

#endif
