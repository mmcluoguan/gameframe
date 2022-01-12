#include "frmpub/basic.h"
#include "3rd/fmt/format.h"
#include "frmpub/logconnectormgr.h"
#include "shynet/utils/stuff.h"

namespace frmpub {

rapidjson::Value& get_json_value(rapidjson::Value& jv, std::string key)
{
    auto iter = jv.FindMember(key.c_str());
    if (iter != jv.MemberEnd()) {
        return iter->value;
    }
    THROW_EXCEPTION("找不到json key:" + key);
}

rapidjson::Value& get_json_value(rapidjson::Document& doc, std::string key)
{
    auto iter = doc.FindMember(key.c_str());
    if (iter != doc.MemberEnd()) {
        return iter->value;
    }
    THROW_EXCEPTION("找不到json key:" + key);
}

/**
 * @brief 设置日志收集
*/
void set_loggather()
{
    static bool flag = false;
    if (flag == false) {
        shynet::utils::Logger::setoutput([](const char* msg, size_t len) {
            static char processname[NAME_MAX] = { 0 };
            if (shynet::utils::Singleton<frmpub::LogConnectorMgr>::exist_instance()) {
                std::shared_ptr<LogConnector> logger = shynet::utils::Singleton<frmpub::LogConnectorMgr>::instance().log_connector();
                if (strlen(processname) == 0) {
                    char path[PATH_MAX] = { 0 };
                    shynet::utils::stuff::executable_path(path, processname, sizeof(path));
                    char* processname_end = strrchr(processname, '.');
                    if (processname != nullptr && processname_end != nullptr) {
                        *processname_end = '\0';
                    }
                }
                frmpub::protocc::writelog_to_log_c msgc;
                msgc.set_dirname(processname);
                msgc.set_logname(shynet::utils::Logger::logname());
                msgc.set_logdata(msg, len);
                logger->send_proto({ protocc::WRITELOG_TO_LOG_C, &msgc },
                    { protocc::WRITELOG_TO_LOG_S,
                        [&](auto data, auto enves) -> int {
                            return 0;
                        },
                        timeval {} });

                shynet::utils::Logger::print_cout(msg, len);
            }
        });
    }
    flag = true;
}

std::string Basic::internal_msgname(int id)
{
    return "(" + frmpub::protocc::InternalMsgId_Name(id) + ":" + std::to_string(id) + ")";
}

std::string Basic::client_msgname(int id)
{
    return "(" + frmpub::protocc::ClientMsgId_Name(id) + ":" + std::to_string(id) + ")";
}

std::string Basic::json_msgname(int id)
{
    std::string ename(magic_enum::enum_name<JosnMsgId>(static_cast<JosnMsgId>(id)));
    return "(" + ename + ":" + std::to_string(id) + ")";
}

std::string Basic::msgname(int id)
{
    std::string str = client_msgname(id);
    if (str == "(:" + std::to_string(id) + ")") {
        str = internal_msgname(id);
        if (str == "(:" + std::to_string(id) + ")") {
            str = json_msgname(id);
        }
    }
    if (str == "(:" + std::to_string(id) + ")") {
        return fmt::format("(unkonw:{})", id);
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
