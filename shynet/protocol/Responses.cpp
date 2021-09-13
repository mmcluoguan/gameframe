#include "shynet/protocol/Responses.h"
#include <algorithm>
#include <cstring>
#include "shynet/utils/StringOp.h"
#include "shynet/utils/Logger.h"

namespace shynet {
	namespace protocol {
		Responses::Responses() {
		}
		Responses::~Responses() {
		}

		int Responses::responses_uninit(std::string& line) {
			std::string::const_iterator start = line.begin();
			std::string::const_iterator end = line.end();
			std::string::const_iterator space = std::find(start, end, ' ');
			if (space == end) { LOG_WARN << "协议错误"; return -1; }
			version_.assign(start, space);
			start = space + 1;
			space = std::find(start, end, ' ');
			if (space == end) { LOG_WARN << "协议错误"; return -1; }
			else {
				status_code_ = atoi(std::string(start, space).c_str());
				if (status_code_ != 200 && status_code_ != 101) {
					LOG_WARN << "status_code:" << status_code_;
				}
				desc_ = std::string(space, end);
			}
			step_ = Step::INIT;
			return 0;
		}

		int Responses::responses_init(std::string& line) {
			if (line.empty() == false) {
				std::string::size_type end = line.find(":");
				if (end != std::string::npos) {
					std::string key = line.substr(0, end);
					key = utils::StringOp::trim(key);
					std::string value = line.substr(end + 1);
					value = utils::StringOp::trim(value);
					heads_[key] = value;
				}
				else {
					LOG_WARN << "协议错误"; return -1;
				}
			}
			else {
				step_ = Step::Parse;
			}
			return 0;
		}

		size_t Responses::data_length() const {
			auto it = heads_.find("Content-Length");
			if (it == heads_.end()) {
				return 0L;
			}
			else {
				char* p;
				size_t len = strtoull(it->second.c_str(), &p, 10);
				if (p == nullptr) {
					LOG_WARN << "Content-Length的值不是数字";
					return 0L;
				}
				return len;
			}
		}

		const char* Responses::websocket_key() const {
			const auto& it = heads_.find("Upgrade");
			if (it == heads_.end()) {
				return nullptr;
			}
			else {
				if (strcasecmp(it->second.c_str(), "websocket") == 0) {
					const auto& key = heads_.find("Sec-WebSocket-Accept");
					if (key == heads_.end()) {
						return nullptr;
					}
					else {
						return key->second.c_str();
					}
				}
				else {
					return nullptr;
				}
			}
		}
	}
}
