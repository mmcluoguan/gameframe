#pragma once
#include <map>
#include <memory>
#include "shynet/events/Streambuff.h"
#include "shynet/Basic.h"
#include "shynet/protocol/Request.h"
#include "shynet/protocol/Responses.h"

namespace shynet {

	namespace protocol {
		class FilterProces;
		class WebSocket : public Nocopy {
		public:
			enum class FrameType {
				Continuation,
				Text,
				Binary,
				Close = 8,
				Ping = 9,
				Pong = 10,
			};

			enum class Status {
				Unconnect = 0,
				Handsharked,
			};

			explicit WebSocket(FilterProces* filter);
			~WebSocket();

			int process();
			int process_requset(std::shared_ptr<events::Streambuff> inputbuffer,
				std::shared_ptr<events::Streambuff> restore);
			int process_responses(std::shared_ptr<events::Streambuff> inputbuffer,
				std::shared_ptr<events::Streambuff> restore);
			int process_data(std::shared_ptr<events::Streambuff> inputbuffer,
				std::shared_ptr<events::Streambuff> restore);

			int upgrade(const char* key);
			int verify(const char* key);
			int request_handshake();

			int send1(const void* data, size_t len, FrameType op) const;
			int send(const void* data, size_t len, FrameType op) const;
			int send(std::string data, FrameType op) const;
		private:
		private:
			FilterProces* filter_;
			Request requset_;
			Responses responses_;
			const char* magic_key_{ "258EAFA5-E914-47DA-95CA-C5AB0DC85B11" };
			Status status_ = Status::Unconnect;
			std::string request_key_;
			//最大接收缓冲大小
			std::unique_ptr<char[]> total_original_data_;
			size_t total_postion_ = 0;
		};
	}
}
