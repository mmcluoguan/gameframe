#include "shynet/utils/StringOp.h"
#include <arpa/inet.h>
#include <cstring>
#include <sys/resource.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>

namespace shynet {
	namespace utils {
		

		std::string& StringOp::trim(std::string& xstr, std::function<int(int)> cb) {
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



		char* StringOp::trim(char* str, std::function<int(int)> cb) {
			char* p = str;
			char* p1;
			if (p) {
				p1 = p + strlen(str) - 1;
				while (*p && cb(*p)) p++;
				while (p1 > p && cb(*p1)) *p1-- = '\0';
			}
			return p;
		}
		int StringOp::spilt(char* target, const char* c, char** argv, int size) {
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

		std::vector<std::string> StringOp::spilt(const std::string& target, const char* c) {
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
	}
}
