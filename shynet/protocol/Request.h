#pragma once
#include "shynet/Basic.h"
#include <map>

namespace shynet {
	namespace protocol {
		class Request : public Nocopy {
		public:
			Request();
			~Request();

			int requset_uninit(std::string& line);
			int requset_init(std::string& line);
			Step step() const {
				return step_;
			}
			size_t data_length() const;
			const char* websocket_key() const;
		private:
			std::string method_;
			std::string version_;
			std::string path_;
			std::map<std::string, std::string> params_;
			std::map<std::string, std::string> heads_;
			Step step_ = Step::UNINIT;
		};
	}
}
