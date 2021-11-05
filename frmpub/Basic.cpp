#include "frmpub/basic.h"
#include "shynet/utils/logger.h"

namespace frmpub {

std::string Basic::internal_msgname(int id)
{
    return "(" + frmpub::protocc::InternalMsgId_Name(id) + ":" + std::to_string(id) + ")";
}

std::string Basic::client_msgname(int id)
{
    return "(" + frmpub::protocc::ClientMsgId_Name(id) + ":" + std::to_string(id) + ")";
}

std::string Basic::msgname(int id)
{
    std::string str = client_msgname(id);
    if (str == "(:" + std::to_string(id) + ")") {
        str = internal_msgname(id);
    }
    if (str == "(:" + std::to_string(id) + ")") {
        return "(unkonw:unkonw)";
    }
    return str;
}

std::string Basic::connectname(protocc::ServerType st)
{
    std::string name = frmpub::protocc::ServerType_Name(st);
    if (name.empty()) {
        return "[unkonw]";
    } else {
        return "[" + name + "]";
    }
}
std::string Sms::send(std::string url)
{
    std::string response;
    CURL* curl = curl_easy_init();
    CURLcode res;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // url
    curl_easy_setopt(curl, CURLOPT_POST, 0); // get reqest
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, req_reply);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&response);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10); // set transport and time out time
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    // start request
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK) {
        LOG_WARN << curl_easy_strerror(res);
        return "";
    }
    return response;
}
size_t Sms::req_reply(void* ptr, size_t size, size_t nmemb, void* stream)
{
    if (stream == NULL || ptr == NULL || size == 0)
        return 0;

    size_t realsize = size * nmemb;
    std::string* buffer = (std::string*)stream;
    if (buffer != NULL) {
        buffer->append((const char*)ptr, realsize);
    }
    return realsize;
}
}
