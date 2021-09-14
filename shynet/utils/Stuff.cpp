#include "shynet/utils/Stuff.h"
#include <arpa/inet.h>
#include <cstring>
#include <sys/resource.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <sstream>
#include <iomanip>
#include <sys/time.h>
#include <netdb.h>
#include "shynet/utils/Logger.h"

namespace shynet {
	namespace utils {
		void Stuff::create_coredump() {
			struct rlimit core { RLIM_INFINITY, RLIM_INFINITY };
			if (setrlimit(RLIMIT_CORE, &core) == -1) {
				LOG_SYSERR << "call setrlimit";
			}
		}

		int Stuff::daemon() {
			if (::daemon(1, 0) == -1) {
				LOG_SYSERR << "call daemon";
			}
			return getpid();
		}

		void Stuff::writepid(const std::string& pidfile) {
			writepid(pidfile.c_str());
		}

		void Stuff::writepid(const char* pidfile)
		{
			bool isfree = false;
			if (pidfile == nullptr)
			{
				char path[PATH_MAX] = { 0 };
				char processname[NAME_MAX] = { 0 };
				if (Stuff::executable_path(path, processname, sizeof(path)) == -1) {
					LOG_SYSERR << "call executable_path";
				}
				char* processname_end = strrchr(processname, '.');
				if (processname != nullptr) {
					*processname_end = '\0';
				}
				pidfile = new char[NAME_MAX];
				sprintf(const_cast<char*>(pidfile), "./%s.pid", processname);
				isfree = true;
			}
			int fp = open(pidfile, O_WRONLY | O_CREAT | O_TRUNC, 0600);
			if (isfree) {
				delete[] pidfile;
			}
			if (fp == -1) {
				LOG_SYSERR << "call open";
			}
			if (lockf(fp, F_TLOCK, 0) < 0) {
				LOG_SYSERR << "call lockf";
			}

			char buf[64] = { 0 };
			pid_t pid = ::getpid();
			snprintf(buf, 64, "%d\n", pid);
			size_t len = strlen(buf);
			::write(fp, buf, len);
			close(fp);
		}
		
		int Stuff::executable_path(char* processdir, char* processname, size_t len) {
			char* path_end;
			if (readlink("/proc/self/exe", processdir, len) <= 0)
				return -1;
			path_end = strrchr(processdir, '/');
			if (path_end == NULL)
				return -1;
			++path_end;
			strcpy(processname, path_end);
			*path_end = '\0';
			return static_cast<int>(path_end - processdir);
		}
		void Stuff::random(void* buf, size_t len) {
			bool done = false;
			FILE* fp = fopen("/dev/urandom", "rb");
			if (fp != NULL) {
				fread(buf, 1, len, fp);
				fclose(fp);
				done = true;
			}
			if (!done) {
				size_t i;
				for (i = 0; i < len; i++) {
					((unsigned char*)buf)[i] = (unsigned char)(rand() % 0xff);
				}
			}
		}

		uint64_t Stuff::hl64ton(uint64_t host) {
			uint64_t ret = 0;
			uint32_t high, low;
			low = (uint32_t)(host & 0xFFFFFFFF);
			high = (uint32_t)((host >> 32) & 0xFFFFFFFF);

			low = htonl(low);
			high = htonl(high);

			ret = low;
			ret <<= 32;
			ret |= high;
			return ret;

		}

		uint64_t Stuff::ntohl64(uint64_t host) {
			uint64_t ret = 0;
			uint32_t high, low;

			low = (uint32_t)(host & 0xFFFFFFFF);
			high = (uint32_t)((host >> 32) & 0xFFFFFFFF);

			low = ntohl(low);
			high = ntohl(high);

			ret = low;
			ret <<= 32;
			ret |= high;
			return ret;
		}
		
		std::string Stuff::readable_bytes(uint64_t n)
		{
			char UNITS[] = { 'B', 'K', 'M', 'G', 'T', 'P', 'E' }; // 'Z' 'Y'
			int index = 0;
			for (; n >> (index * 10); index++) {
				if (index == 6) { /// can't do n >> 70
					++index;
					break;
				}
			}
			index = index > 0 ? index - 1 : index;
			char buf[128] = { 0 };
			snprintf(buf, 127, "%.1f%c",
				static_cast<float>(n) / static_cast<float>(index ? (1UL << (index * 10)) : 1),
				UNITS[index]
			);
			return std::string(buf);
		}
		
		std::string Stuff::bytes_to_hex(const uint8_t* buf, std::size_t len, std::size_t num_per_line, bool with_ascii)
		{
			if (!buf || len == 0 || num_per_line == 0) { return std::string(); }

			static const std::string PRINTABLE_ASCII = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~ ";

			std::size_t num_of_line = len / num_per_line;
			if (len % num_per_line) { num_of_line++; }

			std::ostringstream oss;
			for (std::size_t i = 0; i < num_of_line; i++) {
				std::size_t item = (len % num_per_line) && (i == num_of_line - 1) ? (len % num_per_line) : num_per_line;
				std::ostringstream line_oss;
				for (std::size_t j = 0; j < item; j++) {
					if (j != 0) { line_oss << ' '; }

					line_oss << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(buf[i * num_per_line + j]);
				}

				if ((len % num_per_line) && (i == num_of_line - 1) && with_ascii) {
					oss << std::left << std::setw((int)(num_per_line * 2 + num_per_line - 1)) << line_oss.str();
				}
				else {
					oss << line_oss.str();
				}

				if (with_ascii) {
					oss << "    ";
					for (std::size_t j = 0; j < item; j++) {
						if (PRINTABLE_ASCII.find(buf[i * num_per_line + j]) != std::string::npos) {
							oss << buf[i * num_per_line + j];
						}
						else {
							oss << '.';
						}
					}
				}

				if (i != num_of_line) { oss << '\n'; }
			}

			return oss.str();
		}
		
		std::string Stuff::gethostbyname(const char* domain)
		{
			struct hostent* ht = ::gethostbyname(domain);
			if (ht == NULL || ht->h_length <= 0) { return std::string(); }

			char result[64] = { 0 };
			snprintf(result, 63, "%hhu.%hhu.%hhu.%hhu",
				static_cast<uint8_t>(ht->h_addr_list[0][0]),
				static_cast<uint8_t>(ht->h_addr_list[0][1]),
				static_cast<uint8_t>(ht->h_addr_list[0][2]),
				static_cast<uint8_t>(ht->h_addr_list[0][3])
			);
			return std::string(result);
		}
		
		uint64_t Stuff::tick_msec()
		{
			struct timespec ts;
			clock_gettime(CLOCK_MONOTONIC, &ts);
			return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
		}
		
		uint64_t Stuff::unix_timestamp_msec()
		{
			struct timeval tv;
			gettimeofday(&tv, NULL);
			return tv.tv_sec * 1000 + tv.tv_usec / 1000;
		}
	}
}
