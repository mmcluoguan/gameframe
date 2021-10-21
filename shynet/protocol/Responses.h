#ifndef SHYNET_PROTOCOL_RESPONSES_H
#define SHYNET_PROTOCOL_RESPONSES_H

#include "shynet/Basic.h"
#include <map>

namespace shynet {
namespace protocol {
    class Responses : public Nocopy {
    public:
        Responses();
        ~Responses();

        int responses_uninit(std::string& line);
        int responses_init(std::string& line);
        Step step() const
        {
            return step_;
        }
        size_t data_length() const;
        const char* websocket_key() const;

    private:
        std::string method_;
        std::string version_;
        int status_code_ = 200;
        std::string desc_;
        std::map<std::string, std::string> heads_;
        Step step_ = Step::UNINIT;
    };
}
}

#endif
