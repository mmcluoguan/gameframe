#ifndef SHYNET_UTILS_LOGGER_H
#define SHYNET_UTILS_LOGGER_H

#include "shynet/basic.h"
#include <map>
#include <memory>
#include <sstream>

namespace shynet {
namespace utils {
    /**
     * @brief ��־��¼��
    */
    class Logger final : public Nocopy {
    public:
        /**
         * @brief ��־��¼�ص�
        */
        using OutputFunc = void (*)(const char* msg, size_t len);
        /**
         * @brief ��־�ȼ�
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
         * @brief ��־�����ļ�����
        */
        enum class LogPrecise {
            /**
             * @brief ÿСʱһ���ļ�
            */
            HOUR,
            /**
             * @brief ÿ��һ���ļ�
            */
            DAY,
        };

        /**
         * @brief ����
         * @param sourcefile ������־��Դ�����ļ���
         * @param line ������־����Դ������
         * @param level ��־�ȼ�
         * @param fun ������־����Դ������ú���
         * @param savedErrno ϵͳ��������ʧ�ܵĴ�����
        */
        Logger(const char* sourcefile, int line, Logger::LogLevel level = Logger::LogLevel::INFO, const char* fun = nullptr, int savedErrno = 0);
        ~Logger();

        /**
         * @brief ��ȡ�����
         * @return �����
        */
        std::ostream& stream() { return ostream_; }

        /**
         * @brief д��־
         * @param buffer ָ�����ݵ�ָ�� 
         * @param size ָ�����ݵ�ָ���С
        */
        void writelog(const unsigned char* buffer, size_t size);

    public:
        /**
         * @brief �����Ϣ��cout
         * @param msg ��Ϣ
         * @param len ����
        */
        static void print_cout(const char* msg, size_t len);
        /**
         * @brief ������־�ص�����
         * @param  ��־�ص�����
        */
        static void setoutput(OutputFunc out);
        /**
         * @brief ��ȡ��־�ȼ�
         * @return ��־�ȼ�
        */
        static Logger::LogLevel loglevel();
        /**
         * @brief ������־�ȼ�
         * @param level ��־�ȼ�
        */
        static void set_loglevel(Logger::LogLevel level);
        /**
         * @brief ��ȡ��־�ļ���ǰ׺
         * @return ��־�ļ���ǰ׺
        */
        static std::string logname();
        /**
         * @brief ������־�ļ���ǰ׺
         ���û�����ã�Ĭ��ʹ��Ӧ�ó�������
         * @param name ��־�ļ���ǰ׺
        */
        static void set_logname(std::string name);
        /**
         * @brief ��ȡ��־�����ļ�����
         * @return ��־�����ļ�����
        */
        static Logger::LogPrecise logprecise();
        /**
         * @brief ������־�����ļ�����
         * @param percise ��־�����ļ�����
        */
        static void set_logprecise(Logger::LogPrecise percise);

    private:
        /**
         * @brief �����
        */
        std::ostringstream ostream_;
        /**
         * @brief ������־��Դ�����ļ���
        */
        const char* sourcefile_;
        /**
         * @brief ������־����Դ������
        */
        int line_;
        /**
         * @brief ��־�ȼ�
        */
        Logger::LogLevel level_;
        /**
         * @brief ������־����Դ������ú���
        */
        const char* fun_;

    private:
        /**
         * @brief ȫ����־�ȼ�
        */
        inline static LogLevel g_level_ = LogLevel::DEBUG;
        /**
         * @brief ȫ����־�ص�����
        */
        static OutputFunc g_output_;
        /**
         * @brief ָ����־ǰ׺����ָ��
        */
        inline static std::unique_ptr<std::string> g_logname_;
        /**
         * @brief ȫ����־�����ļ�����
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
