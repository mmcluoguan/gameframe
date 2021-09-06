#include "shynet/Utility.h"
#include <arpa/inet.h>
#include <cstring>
#include <sys/resource.h>
#include <unistd.h>
#include "shynet/Logger.h"

namespace shynet {
	void Utility::create_coredump() {
		struct rlimit core { RLIM_INFINITY, RLIM_INFINITY };
		if (setrlimit(RLIMIT_CORE, &core) == -1) {
			LOG_SYSERR << "call setrlimit";
		}
	}
	void Utility::daemon() {
		if (::daemon(1, 0) == -1) {
			LOG_SYSERR << "call daemon";
		}
	}

	std::string& Utility::trim(std::string& xstr, std::function<int(int)> cb) {
		if (xstr.empty()) {
			return xstr;
		}

		std::string::iterator c = xstr.begin();
		for (; c != xstr.end() && cb(*c++););
		xstr.erase(xstr.begin(), --c);

		for (c = xstr.end(); c != xstr.begin() && cb(*--c););
		xstr.erase(++c, xstr.end());

		return xstr;
	}

	char* Utility::trim(char* str, std::function<int(int)> cb) {
		char* p = str;
		char* p1;
		if (p) {
			p1 = p + strlen(str) - 1;
			while (*p && cb(*p)) p++;
			while (p1 > p && cb(*p1)) *p1-- = '\0';
		}
		return p;
	}
	int Utility::spilt(char* target, const char* c, char** argv, int size) {
		int i = 0;
		char* p = NULL;
		char* tp;
		p = strtok_r(target, c, &tp);
		while (p && i < size) {
			argv[i] = p;
			++i;
			p = strtok_r(NULL, c, &tp);
		}
		return i;
	}
	std::vector<std::string> Utility::spilt(const std::string& target, const char* c) {
		std::vector<std::string> vts;
		size_t last = 0;
		size_t index = target.find_first_of(c, last);
		while (index != std::string::npos) {
			vts.push_back(target.substr(last, index - last));
			last = index + 1;
			index = target.find_first_of(c, last);
		}
		if (index - last > 0) {
			vts.push_back(target.substr(last, index - last));
		}
		return vts;
	}
	int Utility::get_executable_path(char* processdir, char* processname, size_t len) {
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
	void Utility::random(void* buf, size_t len) {
		bool done = false;
		FILE* fp = fopen("/dev/urandom", "rb");
		if (fp != NULL) {
			fread(buf, 1, len, fp);
			fclose(fp);
			done = true;
		}
		if (!done) {
			// Fallback to a pseudo random gen
			size_t i;
			for (i = 0; i < len; i++) {
				((unsigned char*)buf)[i] = (unsigned char)(rand() % 0xff);
			}
		}
	}

	uint64_t Utility::hl64ton(uint64_t host) {
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

	uint64_t Utility::ntohl64(uint64_t host) {
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
}
