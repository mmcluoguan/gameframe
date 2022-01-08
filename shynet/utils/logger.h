#ifndef SHYNET_UTILS_LOGGER_H
#define SHYNET_UTILS_LOGGER_H

#include "shynet/basic.h"
#include <map>
#include <memory>
#include <sstream>

namespace shynet {
namespace utils {
    /**
     * @brief 日志记录器
    */
    class Logger final : public Nocopy {
    public:
        /**
         * @brief 日志记录回调
        */
        using OutputFunc = void (*)(const char* msg, size_t len);
        /**
         * @brief 日志等级
        */
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

        /**
         * @brief 日志生成文件精度
        */
        enum class LogPrecise {
            /**
             * @brief 每小时一个文件
            */
            HOUR,
            /**
             * @brief 每天一个文件
            */
            DAY,
        };

        /**
         * @brief 构造
         * @param sourcefile 产生日志所源代码文件名
         * @param line 产生日志所在源代码行
         * @param level 日志等级
         * @param fun 产生日志所在源代码调用函数
         * @param savedErrno 系统函数调用失败的错误码
        */
        Logger(const char* sourcefile, int line, Logger::LogLevel level = Logger::LogLevel::INFO, const char* fun = nullptr, int savedErrno = 0);
        ~Logger();

        /**
         * @brief 获取输出流
         * @return 输出流
        */
        std::ostream& stream() { return ostream_; }

        /**
         * @brief 写日志
         * @param buffer 指向数据的指针 
         * @param size 指向数据的指针大小
        */
        void writelog(const unsigned char* buffer, size_t size);

    public:
        /**
         * @brief 输出信息到cout
         * @param msg 信息
         * @param len 长度
        */
        static void print_cout(const char* msg, size_t len);
        /**
         * @brief 设置日志回调函数
         * @param  日志回调函数
        */
        static void setoutput(OutputFunc out);
        /**
         * @brief 获取日志等级
         * @return 日志等级
        */
        static Logger::LogLevel loglevel();
        /**
         * @brief 设置日志等级
         * @param level 日志等级
        */
        static void set_loglevel(Logger::LogLevel level);
        /**
         * @brief 获取日志文件名前缀
         * @return 日志文件名前缀
        */
        static std::string logname();
        /**
         * @brief 设置日志文件名前缀
         如果没有设置，默认使用应用程序名称
         * @param name 日志文件名前缀
        */
        static void set_logname(std::string name);
        /**
         * @brief 获取日志生成文件精度
         * @return 日志生成文件精度
        */
        static Logger::LogPrecise logprecise();
        /**
         * @brief 设置日志生成文件精度
         * @param percise 日志生成文件精度
        */
        static void set_logprecise(Logger::LogPrecise percise);

    private:
        /**
         * @brief 输出流
        */
        std::ostringstream ostream_;
        /**
         * @brief 产生日志所源代码文件名
        */
        const char* sourcefile_;
        /**
         * @brief 产生日志所在源代码行
        */
        int line_;
        /**
         * @brief 日志等级
        */
        Logger::LogLevel level_;
        /**
         * @brief 产生日志所在源代码调用函数
        */
        const char* fun_;

    private:
        /**
         * @brief 全局日志等级
        */
        inline static LogLevel g_level_ = LogLevel::DEBUG;
        /**
         * @brief 全局日志回调函数
        */
        static OutputFunc g_output_;
        /**
         * @brief 指向日志前缀名的指针
        */
        inline static std::unique_ptr<std::string> g_logname_;
        /**
         * @brief 全局日志生成文件精度
        */
        inline static LogPrecise g_precise_ = LogPrecise::DAY;
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
