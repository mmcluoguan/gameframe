#include "shynet/protocol/Request.h"
#include "shynet/utils/Logger.h"
#include "shynet/utils/StringOp.h"
#include <cstring>

namespace shynet {
namespace protocol {
    Request::Request()
    {
    }
    Request::~Request()
    {
    }

    size_t Request::data_length() const
    {
        auto it = heads_.find("Content-Length");
        if (it == heads_.end()) {
            return 0L;
        } else {
            char* p;
            size_t len = strtoull(it->second.c_str(), &p, 10);
            if (*p != '\0') {
                LOG_WARN << "Content-Length的值不是数字";
                return 0L;
            }
            return len;
        }
    }

    const char* Request::websocket_key() const
    {
        const auto& it = heads_.find("Upgrade");
        if (it == heads_.end()) {
            return nullptr;
        } else {
            if (strcasecmp(it->second.c_str(), "websocket") == 0) {
                const auto& key = heads_.find("Sec-WebSocket-Key");
                if (key == heads_.end()) {
                    return nullptr;
                } else {
                    return key->second.c_str();
                }
            } else {
                return nullptr;
            }
        }
    }

    int Request::requset_uninit(std::string& line)
    {
        std::string::const_iterator start = line.begin();
        std::string::const_iterator end = line.end();
        std::string::const_iterator space = std::find(start, end, ' ');
        if (space == end) {
            LOG_WARN << "协议错误";
            return -1;
        }
        method_.assign(start, space);
        start = space + 1;
        space = std::find(start, end, ' ');
        if (space == end) {
            LOG_WARN << "协议错误";
            return -1;
        }
        std::string::const_iterator question = std::find(start, space, '?');
        if (question != space) {
            path_.assign(start, question);
            std::string::const_iterator deng = std::find(question, space, '=');
            while (deng != space) {
                std::string key(question + 1, deng);
                std::string::const_iterator yu = std::find(deng, space, '&');
                question = yu;
                std::string value;
                if (yu != space) {
                    value.assign(deng + 1, yu);
                } else {
                    value.assign(deng + 1, space);
                }
                params_[key] = value;
                deng = std::find(yu, space, '=');
            }
        } else {
            path_.assign(start, question);
        }
        start = space + 1;
        version_.assign(start, end);
        step_ = Step::INIT;
        return 0;
    }

    int Request::requset_init(std::string& line)
    {
        if (line.empty() == false) {
            std::string::size_type end = line.find(":");
            if (end != std::string::npos) {
                std::string key = line.substr(0, end);
                key = utils::StringOp::trim(key);
                std::string value = line.substr(end + 1);
                value = utils::StringOp::trim(value);
                heads_[key] = value;
            } else {
                LOG_WARN << "协议错误";
                return -1;
            }
        } else {
            step_ = Step::Parse;
        }
        return 0;
    }
}
}
