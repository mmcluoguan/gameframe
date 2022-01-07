#include "log/logtimer.h"
#include "3rd/fmt/format.h"
#include "shynet/net/timerreactormgr.h"
#include "shynet/utils/elapsed.h"
#include "shynet/utils/filepathop.h"
#include "shynet/utils/iniconfig.h"
#include "shynet/utils/singleton.h"

extern const char* g_conf_node;

namespace logs {
LogTimer::LogTimer(const timeval val)
    : net::TimerEvent(val, EV_TIMEOUT | EV_PERSIST)
{
}
void LogTimer::timeout()
{
    shynet::utils::IniConfig& ini = shynet::utils::Singleton<shynet::utils::IniConfig>::get_instance();
    time_t t = time(nullptr);
    struct tm tm_time;
    localtime_r(&t, &tm_time);
    char timebuf[30] = { 0 };
    std::string precise = ini.get<std::string>(g_conf_node, "precise");
    if (precise == "hour") {
        strftime(timebuf, sizeof(timebuf), "%F_%H.log", &tm_time);
    } else {
        strftime(timebuf, sizeof(timebuf), "%F.log", &tm_time);
    }
    auto cb = [&]() {
        std::lock_guard<std::mutex> lock(data_mut_);
        for (auto&& [key, value] : data_) {
            std::string dir = fmt::format("./log/{}/", key.dirname);
            if (shynet::utils::filepathop::exist(dir) != 0) {
                shynet::utils::filepathop::mkdir_recursive(dir);
            }
            std::string logfilename = fmt::format("{}/{}_{}", dir, key.logname, timebuf);
            std::ofstream logfile;
            logfile.open(logfilename, std::ios::out | std::ios::app);
            for (auto& it : value) {

                logfile << it.logdata() << std::endl;
            }
            value.clear();
        }
    };
#ifdef USE_DEBUG
    shynet::utils::elapsed("工作线程计时单任务执行 LogTimer");
    return cb();
#else
    return cb();
#endif
}
void LogTimer::add(const frmpub::protocc::writelog_to_log_c& data)
{
    std::lock_guard<std::mutex> lock(data_mut_);
    auto it = data_.find({ data.dirname(),
        data.logname() });
    if (it == data_.end()) {
        std::list<frmpub::protocc::writelog_to_log_c> ls;
        ls.emplace_back(data);
        data_.insert({ { data.dirname(),
                           data.logname() },
            ls });
    } else {
        it->second.emplace_back(data);
    }
}
}
