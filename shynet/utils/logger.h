#ifndef SHYNET_UTILS_LOGGER_H
#define SHYNET_UTILS_LOGGER_H

#include "shynet/basic.h"
#include <memory>
#include <sstream>

namespace shynet {
namespace utils {
    class Logger final : public Nocopy {
    public:
        using OutputFunc = void (*)(const char* msg, size_t len);
        enum class LogLevel {
            TRACE,
            DEBUG,
            LUA,
            INFO,
            WARN,
            ERROR,
            FATAL,
            NUM_LOG_LEVELS,
        };

        Logger(const char* sourcefile, int line, Logger::LogLevel level = Logger::LogLevel::INFO, const char* fun = nullptr, int savedErrno = 0);
        ~Logger();
        std::ostream& stream()
        {
            return ostream_;
        }

        void writelog(const unsigned char* buffer, size_t size);

    public:
        static void setoutput(OutputFunc);
        static Logger::LogLevel loglevel();
        static void set_loglevel(Logger::LogLevel level);
        static std::string logname();
        static void set_logname(std::string name);

    private:
        std::ostringstream ostream_;
        const char* sourcefile_;
        int line_;
        Logger::LogLevel level_;
        const char* fun_;

    private:
        static Logger::LogLevel g_level_;
        static OutputFunc g_output_;
        inline static std::unique_ptr<std::string> g_logname_;
    };
}
}

#undef LOG_TRACE
#undef LOG_DEBUG
#undef LOG_INFO
#undef LOG_WARN
#undef LOG_ERROR
#undef LOG_FATAL
#undef LOG_SYSERR

#define LOG_TRACE                                                                    \
    if (shynet::utils::Logger::loglevel() <= shynet::utils::Logger::LogLevel::TRACE) \
    shynet::utils::Logger(__FILE__, __LINE__, shynet::utils::Logger::LogLevel::TRACE, __func__).stream()
#define LOG_DEBUG                                                                    \
    if (shynet::utils::Logger::loglevel() <= shynet::utils::Logger::LogLevel::DEBUG) \
    shynet::utils::Logger(__FILE__, __LINE__, shynet::utils::Logger::LogLevel::DEBUG, __func__).stream()
#define LOG_LUA                                                                    \
    if (shynet::utils::Logger::loglevel() <= shynet::utils::Logger::LogLevel::LUA) \
    shynet::utils::Logger(nullptr, 0, shynet::utils::Logger::LogLevel::LUA).stream()
#define LOG_INFO                                                                    \
    if (shynet::utils::Logger::loglevel() <= shynet::utils::Logger::LogLevel::INFO) \
    shynet::utils::Logger(__FILE__, __LINE__).stream()
#define LOG_INFO_BASE                                                               \
    if (shynet::utils::Logger::loglevel() <= shynet::utils::Logger::LogLevel::INFO) \
    shynet::utils::Logger(nullptr, 0).stream()
#define LOG_WARN shynet::utils::Logger(__FILE__, __LINE__, shynet::utils::Logger::LogLevel::WARN).stream()
#define LOG_WARN_BASE shynet::utils::Logger(nullptr, 0, shynet::utils::Logger::LogLevel::WARN).stream()
#define LOG_ERROR shynet::utils::Logger(__FILE__, __LINE__, shynet::utils::Logger::LogLevel::ERROR).stream()
#define LOG_FATAL shynet::utils::Logger(__FILE__, __LINE__, shynet::utils::Logger::LogLevel::FATAL).stream()
#define LOG_SYSERR shynet::utils::Logger(__FILE__, __LINE__, shynet::utils::Logger::LogLevel::ERROR, nullptr, errno).stream()
#define LOG_SYSFATAL shynet::utils::Logger(__FILE__, __LINE__, shynet::utils::Logger::LogLevel::FATAL, nullptr, errno).stream()
#define LOG_DEBUG_BIN(x, l)                                                          \
    if (shynet::utils::Logger::loglevel() <= shynet::utils::Logger::LogLevel::DEBUG) \
    shynet::utils::Logger(__FILE__, __LINE__, shynet::utils::Logger::LogLevel::DEBUG, __func__).writelog((x), (l))

#endif
