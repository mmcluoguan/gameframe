#include "shynet/utils/logger.h"
#include "shynet/utils/filepathop.h"
#include "shynet/utils/stuff.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits.h>
#include <map>
#include <mutex>
#include <sys/stat.h>
#include <sys/time.h>
#include <thread>
#include <unistd.h>

namespace shynet {
namespace utils {
    struct LogInfo {
        const char* name;
        const char* color;
    };

    static bool g_islogdir = false;
    static std::mutex g_logMutex;
    static std::ofstream g_logfile;
    static char g_logfilename[NAME_MAX] = { 0 };

    static std::map<Logger::LogLevel, LogInfo> levelNames()
    {
        return {
            { Logger::LogLevel::TRACE, { "TRACE", "\033[01;36m" } },
            { Logger::LogLevel::DEBUG, { "DEBUG", "\033[01;32m" } },
            { Logger::LogLevel::LUA, { "LUA", "\033[01;37m" } },
            { Logger::LogLevel::INFO, { "INFO", "\033[01;35m" } },
            { Logger::LogLevel::WARN, { "WARN", "\033[01;33m" } },
            { Logger::LogLevel::ERROR, { "ERROR", "\033[01;31m" } },
            { Logger::LogLevel::FATAL, { "FATAL", "\033[01;34m" } },
        };
    }

    static void defaultOutput(const char* msg, size_t len)
    {
        std::lock_guard<std::mutex> lock(g_logMutex);
        char path[PATH_MAX] = { 0 };
        char processname[NAME_MAX] = { 0 };
        utils::stuff::executable_path(path, processname, sizeof(path));
        char* processname_end = strrchr(processname, '.');
        if (processname != nullptr) {
            *processname_end = '\0';
        }

        constexpr const char* kRoot = "./log/%s";
        if (g_islogdir == false) {
            std::string path = stringop::str_format(kRoot, processname);
            filepathop::mkdir_recursive(path);
            g_islogdir = true;
        }
        time_t t = time(nullptr);
        struct tm tm_time;
        localtime_r(&t, &tm_time);
        char timebuf[30] = { 0 };
        if (Logger::logprecise() == Logger::LogPrecise::HOUR) {
            strftime(timebuf, sizeof(timebuf), "%F_%H.log", &tm_time);
        } else {
            strftime(timebuf, sizeof(timebuf), "%F.log", &tm_time);
        }

        char logfilename[NAME_MAX] = { 0 };
        std::string userlogname = Logger::logname();
        if (userlogname.empty()) {
            sprintf(logfilename, "./log/%s/%s_%s", processname, processname, timebuf);
        } else {
            sprintf(logfilename, "./log/%s/%s_%s", processname, userlogname.c_str(), timebuf);
        }

        if (strncmp(g_logfilename, logfilename, strlen(logfilename)) != 0) {
            if (g_logfile.is_open()) {
                g_logfile.close();
            }
            g_logfile.open(logfilename, std::ios::out | std::ios::app);
            strncpy(g_logfilename, logfilename, strlen(g_logfilename));
        }
        g_logfile << msg << std::endl;
        for (auto&& [key, value] : levelNames()) {
            if (strstr(msg, value.name) != nullptr) {
                std::cout << value.color << msg << "\e[0m" << std::endl;
                break;
            }
        }
    }

    Logger::OutputFunc Logger::g_output_ = defaultOutput;

    Logger::Logger(const char* sourcefile, int line, Logger::LogLevel level, const char* fun, int savedErrno)
    {
        sourcefile_ = sourcefile;
        line_ = line;
        level_ = level;
        fun_ = fun;

        if (sourcefile != nullptr) {
            const char* slash = strrchr(sourcefile, '/');
            if (slash) {
                sourcefile_ = slash + 1;
            }
        }

        time_t t = time(nullptr);
        struct tm tm_time;
        localtime_r(&t, &tm_time);
        char buf[20];
        strftime(buf, sizeof(buf), "%F %T", &tm_time);

        struct timeval time;
        gettimeofday(&time, NULL);

        ostream_ << "[" << levelNames()[level].name << "] " << buf << "." << (time.tv_usec / 1000) << " ";
        if (savedErrno != 0) {
            std::error_condition econd = std::system_category().default_error_condition(savedErrno);
            ostream_ << "[" << econd.category().name();
            ostream_ << "(" << econd.value() << ")";
            ostream_ << " " << econd.message() << "] ";
        }
    }

    Logger::~Logger()
    {
        std::string func;
        if (fun_ != nullptr) {
            func.append(" func:").append(fun_);
        }
        std::thread::id id = std::this_thread::get_id();
        if (sourcefile_ != nullptr)
            ostream_ << " -" << sourcefile_ << " line:" << line_ << " tid:" << std::hash<std::thread::id>()(id) << func;
        g_output_(ostream_.str().c_str(), ostream_.str().length());
        if (level_ == Logger::LogLevel::FATAL) {
            abort();
        }
    }

    static const char* ullto4Str(int n)
    {
        static char buf[64 + 1];
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%06u", n);
        return buf;
    }

    static char g_szchar[] = "0123456789abcdef";

    static char* formLog(int& index, char* szbuf, size_t size_buf, const unsigned char* buffer, size_t size)
    {
        size_t len = 0;
        size_t lsize = 0;
        size_t headlen = 0;
        char szhead[64 + 1] = { 0 };
        memset(szhead, 0, sizeof(szhead));
        while (size > lsize && len + 10 < size_buf) {
            if (lsize % 32 == 0) {
                if (0 != headlen) {
                    szbuf[len++] = '\n';
                }

                memset(szhead, 0, sizeof(szhead));
                strncpy(szhead, ullto4Str(index++), strlen(szhead) - 1);
                headlen = strlen(szhead);
                szhead[headlen++] = ' ';

                strcat(szbuf, szhead);
                len += headlen;
            }
            if (lsize % 16 == 0 && 0 != headlen)
                szbuf[len++] = ' ';
            szbuf[len++] = g_szchar[(buffer[lsize] >> 4) & 0xf];
            szbuf[len++] = g_szchar[(buffer[lsize]) & 0xf];
            lsize++;
        }
        szbuf[len++] = '\n';
        szbuf[len++] = '\0';
        return szbuf;
    }

    void Logger::writelog(const unsigned char* buffer, size_t size)
    {
        static const size_t PRINTSIZE = 512;
        char szbuf[PRINTSIZE * 3 + 8];

        size_t lsize = 0;
        size_t lprintbufsize = 0;
        int index = 0;
        ostream_ << " adress[" << (void*)buffer << "] size[" << size << "] \n";
        while (true) {
            memset(szbuf, 0, sizeof(szbuf));
            if (size > lsize) {
                lprintbufsize = (size - lsize);
                lprintbufsize = lprintbufsize > PRINTSIZE ? PRINTSIZE : lprintbufsize;
                formLog(index, szbuf, sizeof(szbuf), buffer + lsize, lprintbufsize);
                ostream_ << szbuf;
                lsize += lprintbufsize;
            } else {
                break;
            }
        }
    }

    void Logger::setoutput(OutputFunc out)
    {
        g_output_ = out;
    }

    Logger::LogLevel Logger::loglevel()
    {
        return g_level_;
    }

    void Logger::set_loglevel(Logger::LogLevel level)
    {
        g_level_ = level;
    }
    std::string Logger::logname()
    {
        return g_logname_ ? g_logname_->c_str() : "";
    }
    void Logger::set_logname(std::string name)
    {
        g_logname_.reset(new std::string(name));
    }
    Logger::LogPrecise Logger::logprecise()
    {
        return g_precise_;
    }
    void Logger::set_logprecise(Logger::LogPrecise percise)
    {
        g_precise_ = percise;
    }
}
}
