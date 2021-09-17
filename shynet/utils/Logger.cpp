#include "shynet/utils/Logger.h"
#include <cstring>
#include <limits.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <map>
#include <mutex>
#include <thread>
#include "shynet/utils/Stuff.h"

namespace shynet {
	namespace utils {
		static constexpr const char* g_ename[] = {
			/*   0 */ "",
			/*   1 */ "EPERM", "ENOENT", "ESRCH", "EINTR", "EIO", "ENXIO",
			/*   7 */ "E2BIG", "ENOEXEC", "EBADF", "ECHILD",
			/*  11 */ "EAGAIN/EWOULDBLOCK", "ENOMEM", "EACCES", "EFAULT",
			/*  15 */ "ENOTBLK", "EBUSY", "EEXIST", "EXDEV", "ENODEV",
			/*  20 */ "ENOTDIR", "EISDIR", "EINVAL", "ENFILE", "EMFILE",
			/*  25 */ "ENOTTY", "ETXTBSY", "EFBIG", "ENOSPC", "ESPIPE",
			/*  30 */ "EROFS", "EMLINK", "EPIPE", "EDOM", "ERANGE",
			/*  35 */ "EDEADLK/EDEADLOCK", "ENAMETOOLONG", "ENOLCK", "ENOSYS",
			/*  39 */ "ENOTEMPTY", "ELOOP", "", "ENOMSG", "EIDRM", "ECHRNG",
			/*  45 */ "EL2NSYNC", "EL3HLT", "EL3RST", "ELNRNG", "EUNATCH",
			/*  50 */ "ENOCSI", "EL2HLT", "EBADE", "EBADR", "EXFULL", "ENOANO",
			/*  56 */ "EBADRQC", "EBADSLT", "", "EBFONT", "ENOSTR", "ENODATA",
			/*  62 */ "ETIME", "ENOSR", "ENONET", "ENOPKG", "EREMOTE",
			/*  67 */ "ENOLINK", "EADV", "ESRMNT", "ECOMM", "EPROTO",
			/*  72 */ "EMULTIHOP", "EDOTDOT", "EBADMSG", "EOVERFLOW",
			/*  76 */ "ENOTUNIQ", "EBADFD", "EREMCHG", "ELIBACC", "ELIBBAD",
			/*  81 */ "ELIBSCN", "ELIBMAX", "ELIBEXEC", "EILSEQ", "ERESTART",
			/*  86 */ "ESTRPIPE", "EUSERS", "ENOTSOCK", "EDESTADDRREQ",
			/*  90 */ "EMSGSIZE", "EPROTOTYPE", "ENOPROTOOPT",
			/*  93 */ "EPROTONOSUPPORT", "ESOCKTNOSUPPORT",
			/*  95 */ "EOPNOTSUPP/ENOTSUP", "EPFNOSUPPORT", "EAFNOSUPPORT",
			/*  98 */ "EADDRINUSE", "EADDRNOTAVAIL", "ENETDOWN", "ENETUNREACH",
			/* 102 */ "ENETRESET", "ECONNABORTED", "ECONNRESET", "ENOBUFS",
			/* 106 */ "EISCONN", "ENOTCONN", "ESHUTDOWN", "ETOOMANYREFS",
			/* 110 */ "ETIMEDOUT", "ECONNREFUSED", "EHOSTDOWN", "EHOSTUNREACH",
			/* 114 */ "EALREADY", "EINPROGRESS", "ESTALE", "EUCLEAN",
			/* 118 */ "ENOTNAM", "ENAVAIL", "EISNAM", "EREMOTEIO", "EDQUOT",
			/* 123 */ "ENOMEDIUM", "EMEDIUMTYPE", "ECANCELED", "ENOKEY",
			/* 127 */ "EKEYEXPIRED", "EKEYREVOKED", "EKEYREJECTED",
			/* 130 */ "EOWNERDEAD", "ENOTRECOVERABLE", "ERFKILL", "EHWPOISON"
		};

		static constexpr  int g_maxename = 133;

		struct LogInfo {
			const char* name;
			const char* color;
		};

		static std::map<Logger::LogLevel, LogInfo> g_levelNames = {
			{Logger::LogLevel::TRACE,{"TRACE","\033[01;36m"}},
			{Logger::LogLevel::DEBUG,{"DEBUG","\033[01;32m"}},
			{Logger::LogLevel::LUA,{"LUA","\033[01;37m"}},
			{Logger::LogLevel::INFO,{"INFO","\033[01;35m"}},
			{Logger::LogLevel::WARN,{"WARN","\033[01;33m"}},
			{Logger::LogLevel::ERROR,{"ERROR","\033[01;31m"}},
			{Logger::LogLevel::FATAL,{"FATAL","\033[01;34m"}},
		};

		static bool g_islogdir = false;
		static std::mutex g_logMutex;
		static std::ofstream g_logfile;
		static char g_logfilename[100] = {};
		static thread_local char t_errnobuf[512] = { 0 };

		static void defaultOutput(const char* msg, size_t len) {
			std::lock_guard<std::mutex> lock(g_logMutex);
			constexpr const char* root = "./log/";
			if (g_islogdir == false) {
				if (access(root, F_OK) == -1) {
					mkdir(root, S_IRWXU);
				}
				g_islogdir = true;
			}
			time_t t = time(nullptr);
			struct tm tm_time;
			localtime_r(&t, &tm_time);
			char timebuf[30] = { 0 };
			strftime(timebuf, sizeof(timebuf), "%F_%H.log", &tm_time);

			char path[PATH_MAX] = { 0 };
			char processname[NAME_MAX] = { 0 };
			if (utils::Stuff::executable_path(path, processname, sizeof(path)) == -1) {
				abort();
			}
			char* processname_end = strrchr(processname, '.');
			if (processname != nullptr) {
				*processname_end = '\0';
			}

			char logfilename[NAME_MAX] = { 0 };
			sprintf(logfilename, "./log/%s_%s", processname, timebuf);

			if (strncmp(g_logfilename, logfilename, strlen(logfilename)) != 0) {
				if (g_logfile.is_open()) {
					g_logfile.close();
				}
				g_logfile.open(logfilename, std::ios::out | std::ios::app);
				strncpy(g_logfilename, logfilename, strlen(logfilename));
			}
			g_logfile << msg << std::endl;
			for (auto&& [key, value] : g_levelNames) {
				if (strstr(msg, value.name) != nullptr) {
					std::cout << value.color << msg << "\e[0m" << std::endl;
					break;
				}
			}
		}



		Logger::OutputFunc Logger::g_output_ = defaultOutput;
		Logger::LogLevel Logger::g_level_ = LogLevel::DEBUG;

		Logger::Logger(const char* sourcefile, int line, Logger::LogLevel level, const char* fun, int savedErrno) {
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

			ostream_ << "[" << g_levelNames[level].name << "] " << buf << "." << (time.tv_usec / 1000) << " ";
			if (savedErrno != 0) {
				ostream_ << "[" << ((savedErrno > 0 && savedErrno <= g_maxename) ? g_ename[savedErrno] : "?UNKNOWN?");
				ostream_ << "(" << ((savedErrno > 0 && savedErrno <= g_maxename) ? savedErrno : -1) << ")";
				strerror_r(savedErrno, t_errnobuf, sizeof t_errnobuf);
				ostream_ << " " << t_errnobuf << "] ";
			}
		}

		Logger::~Logger() {
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
			else if (level_ == Logger::LogLevel::ERROR) {
				exit(EXIT_FAILURE);
			}
		}

		static const char* ullto4Str(int n) {
			static char buf[64 + 1];
			memset(buf, 0, sizeof(buf));
			sprintf(buf, "%06u", n);
			return buf;
		}

		static char g_szchar[] = "0123456789abcdef";

		static char* formLog(int& index, char* szbuf, size_t size_buf, const unsigned char* buffer, size_t size) {
			size_t len = 0;
			size_t lsize = 0;
			size_t headlen = 0;
			char szhead[64 + 1];
			memset(szhead, 0, sizeof(szhead));
			while (size > lsize && len + 10 < size_buf) {
				if (lsize % 32 == 0) {
					if (0 != headlen) {
						szbuf[len++] = '\n';
					}

					memset(szhead, 0, sizeof(szhead));
					strncpy(szhead, ullto4Str(index++), sizeof(szhead) - 1);
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

		void Logger::writelog(const unsigned char* buffer, size_t size) {
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
				}
				else {
					break;
				}
			}
		}

		void Logger::setoutput(OutputFunc out) {
			g_output_ = out;
		}

		Logger::LogLevel Logger::loglevel() {
			return g_level_;
		}

		void Logger::loglevel(Logger::LogLevel level) {
			g_level_ = level;
		}
	}
}
