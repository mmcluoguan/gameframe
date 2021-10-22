#ifndef SHYNET_PROTOCOL_HTTP_H
#define SHYNET_PROTOCOL_HTTP_H

#include "shynet/events/streambuff.h"
#include "shynet/protocol/request.h"
#include "shynet/protocol/responses.h"

namespace shynet {
namespace protocol {
    class FilterProces;

    class Http : public Nocopy {
    public:
        explicit Http(FilterProces* filter);
        ~Http();

        int process();
        int process_requset(std::shared_ptr<events::Streambuff> inputbuffer,
            std::shared_ptr<events::Streambuff> restore);
        int process_responses(std::shared_ptr<events::Streambuff> inputbuffer,
            std::shared_ptr<events::Streambuff> restore);

        int send_responses(const void* data, size_t len,
            std::string server_name = "",
            std::string content_type = "application/json",
            std::map<std::string, std::string>* server_heads = nullptr,
            int status_code = 200,
            std::string status_desc = "OK",
            std::string version = "HTTP/1.1") const;
        int send_responses(std::string data,
            std::string server_name = "",
            std::string content_type = "application/json",
            std::map<std::string, std::string>* server_heads = nullptr,
            int status_code = 200,
            std::string status_desc = "OK",
            std::string version = "HTTP/1.1") const;
        int send_requset(const void* data, size_t len,
            std::string host = "",
            std::string path = "/",
            std::string version = "HTTP/1.1") const;
        int send_requset(std::string data,
            std::string host = "",
            std::string path = "/",
            std::string version = "HTTP/1.1") const;

    private:
    private:
        FilterProces* filter_;
        Request requset_;
        Responses responses_;
    };
}
}

#endif
