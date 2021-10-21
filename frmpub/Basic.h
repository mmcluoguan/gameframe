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
#include "shynet/events/EventBuffer.h"
#include <curl/curl.h>
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
class Basic {
public:
    static std::string msgname(int);
    static std::string connectname(protocc::ServerType st);

private:
    static std::string internal_msgname(int);
    static std::string client_msgname(int);
    static std::unordered_map<int, std::string> internal_msgname_;
    static std::unordered_map<int, std::string> client_msgname_;
    static std::unordered_map<int, std::string> connect_type_;
};

class Sms {
public:
    static std::string send(std::string url);

private:
    static size_t req_reply(void* ptr, size_t size, size_t nmemb, void* stream);
};
}

#endif
