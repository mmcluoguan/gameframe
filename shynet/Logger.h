#pragma once
#include <sstream>
#include "shynet/Basic.h"

namespace shynet {
	class Logger final : public Nocopy {
	public:
		typedef void(*OutputFunc)(const char* msg, size_t len);
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
		std::ostream& stream() {
			return ostream_;
		}

		void writelog(const unsigned char* buffer, size_t size);
	public:
		static void setoutput(OutputFunc);
		static Logger::LogLevel loglevel();
		static void loglevel(Logger::LogLevel level);
	private:
		std::ostringstream ostream_;
		const char* sourcefile_;
		int line_;
		Logger::LogLevel level_;
		const char* fun_;
	private:
		static Logger::LogLevel g_level_;
		static OutputFunc g_output_;
	};
}

#undef LOG_TRACE
#undef LOG_DEBUG
#undef LOG_INFO
#undef LOG_WARN
#undef LOG_ERROR
#undef LOG_FATAL
#undef LOG_SYSERR

#define LOG_TRACE if (shynet::Logger::loglevel() <= shynet::Logger::LogLevel::TRACE) \
	shynet::Logger(__FILE__, __LINE__, shynet::Logger::LogLevel::TRACE, __func__).stream()
#define LOG_DEBUG if (shynet::Logger::loglevel() <= shynet::Logger::LogLevel::DEBUG) \
	shynet::Logger(__FILE__, __LINE__, shynet::Logger::LogLevel::DEBUG, __func__).stream()
#define LOG_LUA if (shynet::Logger::loglevel() <= shynet::Logger::LogLevel::LUA) \
	shynet::Logger(nullptr, 0, shynet::Logger::LogLevel::LUA).stream()
#define LOG_INFO if (shynet::Logger::loglevel() <= shynet::Logger::LogLevel::INFO) \
	shynet::Logger(__FILE__, __LINE__).stream()
#define LOG_WARN shynet::Logger(__FILE__, __LINE__, shynet::Logger::LogLevel::WARN).stream()
#define LOG_ERROR shynet::Logger(__FILE__, __LINE__, shynet::Logger::LogLevel::ERROR).stream()
#define LOG_FATAL shynet::Logger(__FILE__, __LINE__, shynet::Logger::LogLevel::FATAL).stream()
#define LOG_SYSERR shynet::Logger(__FILE__, __LINE__, shynet::Logger::LogLevel::ERROR,nullptr,errno).stream()
#define LOG_SYSFATAL shynet::Logger(__FILE__, __LINE__, shynet::Logger::LogLevel::FATAL,nullptr,errno).stream()
#define LOG_DEBUG_BIN(x,l) if (shynet::Logger::loglevel() <= shynet::Logger::LogLevel::DEBUG) \
	shynet::Logger(__FILE__, __LINE__, shynet::Logger::LogLevel::DEBUG, __func__).writelog((x), (l))
