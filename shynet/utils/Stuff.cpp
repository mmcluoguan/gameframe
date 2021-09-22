#include "shynet/utils/Stuff.h"
#include "shynet/utils/FilePathOp.h"
#include "shynet/utils/Logger.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <fcntl.h>
#include <netdb.h>
#include <sstream>
#include <unistd.h>
#include <iomanip>
#include <cstring>
#include <climits>

namespace shynet {
	namespace utils {
		namespace {
			static const int64_t BOOT_TIMESTAMP = std::time(NULL);

			class netdev {
			public:
				netdev()
					: in_bytes_(0)
					, out_bytes_(0) {
				}

				uint64_t in_bytes_;
				uint64_t out_bytes_;

				bool parse(const std::string& interface) {
					std::string_view content = FilePathOp::read_file("/proc/net/dev", 65535).get();
					if (content.empty()) { return false; }

					std::string key = interface + ": ";
					size_t pos = content.find(key);
					if (pos == std::string::npos) { return false; }

					std::istringstream ss(std::string(content.data() + pos + key.length()));
					int64_t dummy;
					ss >> in_bytes_ >> dummy >> dummy >> dummy >> dummy >> dummy >> dummy >> dummy
						>> out_bytes_;

					return true;
				}
			};

			class proc_stat {
			public:
				std::string string_dummy_;
				char        ch_dummy_;
				int32_t     int32_dummy_;
				int64_t     int64_dummy_;
				int32_t     virt_kbytes_;
				int32_t     res_kbytes_;

				void parse(const char* content, int64_t page_size) {
					(void)content;
					int64_t vsize = 0;
					int64_t res = 0;
					std::istringstream iss(content);
					iss >> int32_dummy_ >> string_dummy_ >> ch_dummy_ >> int32_dummy_ >> int32_dummy_
						>> int32_dummy_ >> int32_dummy_ >> int32_dummy_ >> int32_dummy_
						>> int64_dummy_ >> int64_dummy_ >> int64_dummy_ >> int64_dummy_
						>> int64_dummy_ >> int64_dummy_ >> int64_dummy_ >> int64_dummy_
						>> int64_dummy_ >> int64_dummy_ >> int64_dummy_ >> int64_dummy_ >> int64_dummy_
						>> vsize >> res >> int64_dummy_;
					virt_kbytes_ = static_cast<int32_t>(vsize / 1024);
					res_kbytes_ = static_cast<int32_t>(res * (page_size / 1024));
				}
			};

			static std::unique_ptr<char[]> status() {
				return FilePathOp::read_file("/proc/self/status", 65535);
			}

			static std::unique_ptr<char[]> stat() {
				return FilePathOp::read_file("/proc/self/stat", 65535);
			}
		}

		void Stuff::create_coredump() {
			struct rlimit core { RLIM_INFINITY, RLIM_INFINITY };
			if (setrlimit(RLIMIT_CORE, &core) == -1) {
				THROW_EXCEPTION("call setrlimit");
			}
		}

		int Stuff::daemon() {
			if (::daemon(1, 0) == -1) {
				THROW_EXCEPTION("call daemon");
			}
			return getpid();
		}

		void Stuff::writepid(const std::string& pidfile) {
			writepid(pidfile.c_str());
		}

		void Stuff::writepid(const char* pidfile) {
			bool isfree = false;
			if (pidfile == nullptr) {
				char path[PATH_MAX] = { 0 };
				char processname[NAME_MAX] = { 0 };
				Stuff::executable_path(path, processname, sizeof(path));
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
				close(fp);
				THROW_EXCEPTION("call open");
			}
			if (lockf(fp, F_TLOCK, 0) < 0) {
				close(fp);
				THROW_EXCEPTION("call lockf");
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
			if (readlink("/proc/self/exe", processdir, len) <= 0) {
				THROW_EXCEPTION("call readlink");
				return -1;
			}
			path_end = strrchr(processdir, '/');
			if (path_end == NULL) {
				THROW_EXCEPTION("找不到'/'");
				return -1;
			}
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

		std::string Stuff::readable_bytes(uint64_t n) {
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

		std::string Stuff::bytes_to_hex(const uint8_t* buf, std::size_t len, std::size_t num_per_line, bool with_ascii) {
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

		std::string Stuff::gethostbyname(const char* domain) {
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

		uint64_t Stuff::tick_msec() {
			struct timespec ts;
			clock_gettime(CLOCK_MONOTONIC, &ts);
			return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
		}

		uint64_t Stuff::unix_timestamp_msec() {
			struct timeval tv;
			gettimeofday(&tv, NULL);
			return tv.tv_sec * 1000 + tv.tv_usec / 1000;
		}

		int32_t Stuff::num_of_threads() {
			std::string_view str(status().get());
			std::size_t pos = str.find("Threads:");
			if (pos == std::string_view::npos) {
				return -1;
			}
			return std::atoi(str.data() + pos + 8);
		}

		int64_t Stuff::boot_timestamp() {
			return BOOT_TIMESTAMP;
		}

		int64_t Stuff::up_duration_seconds() {
			return std::time(NULL) - BOOT_TIMESTAMP;
		}

		bool Stuff::obtain_mem_info(mem_info* mi) {
			if (!mi) { return false; }
			proc_stat ps;
			ps.parse(stat().get(), ::sysconf(_SC_PAGE_SIZE));
			mi->virt_kbytes = ps.virt_kbytes_;
			mi->res_kbytes = ps.res_kbytes_;
			return true;
		}
		bool Stuff::net_interfaces(std::set<std::string>* ifs) {
			if (!ifs) { return false; }

			struct ifreq ifr[64] = { 0 };
			struct ifconf ifc = { 0 };

			int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
			if (sock == -1) { return false; }

			ifc.ifc_buf = (char*)ifr;
			ifc.ifc_len = sizeof(ifr);
			if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) { close(sock); return false; }

			struct ifreq* it = ifc.ifc_req;
			struct ifreq* end = reinterpret_cast<struct ifreq*>((char*)ifr + ifc.ifc_len);
			for (; it != end; ++it) {
				ifs->insert(it->ifr_name);
			}

			close(sock);
			return true;
		}
		bool Stuff::net_interface_bytes(const std::string& netname, uint64_t* in, uint64_t* out) {
			netdev nd;
			if (!nd.parse(netname)) { return false; }

			if (in) { *in = nd.in_bytes_; }
			if (out) { *out = nd.out_bytes_; }

			return true;
		}

		void Stuff::print_exception(const std::exception& e, int level) {
			LOG_ERROR << "异常层级: " << level << " 异常信息:" << e.what();
			try {
				if (auto ptr = dynamic_cast<const std::nested_exception*>(&e)) {
					if (ptr->nested_ptr())
						std::rethrow_exception(ptr->nested_ptr());
				}
			}
			catch (const std::exception& e) {
				print_exception(e, level + 1);
			}
			catch (...) {}
		}
	}
}
