#include "shynet/protocol/Tcpip.h"
#include "shynet/protocol/FilterProces.h"
#include "shynet/utils/Logger.h"
#include <jemalloc/jemalloc.h>
#include <cstring>

namespace shynet {
	namespace protocol {
		Tcpip::Tcpip(FilterProces* filter) : 
			filter_(filter),
			total_original_data_(new char[filter_->max_reve_buf_size]) {
		}

		Tcpip::~Tcpip() {
		}

		int Tcpip::process() {
			std::shared_ptr<events::Streambuff> inputbuffer = filter_->iobuf()->inputbuffer();
			std::shared_ptr<events::Streambuff> restore(new events::Streambuff);
			while (inputbuffer->length() > 0) {
				size_t len;
				inputbuffer->lock();
				char* line = inputbuffer->readln(&len, evbuffer_eol_style::EVBUFFER_EOL_CRLF_STRICT);
				restore->add(line, len);
				restore->addprintf("\r\n");
				inputbuffer->unlock();
				if (line == nullptr) {
					je_free(line);
					LOG_WARN << "协议错误";
					return -1;
				}
				else {
					je_free(line);
					FrameType ft;
					size_t needlen = sizeof(char);
					if (inputbuffer->length() >= needlen)
					{
						//读取帧类型
						inputbuffer->lock();
						inputbuffer->remove(&ft, needlen);
						restore->add(&ft, needlen);
						inputbuffer->unlock();

						uint32_t data_len;
						needlen = sizeof(data_len);
						if (inputbuffer->length() >= needlen) {
							//读取长度
							inputbuffer->lock();
							inputbuffer->remove(&data_len, needlen);
							restore->add(&data_len, needlen);
							inputbuffer->unlock();
							size_t data_length = ntohl(data_len);
							if (inputbuffer->length() >= data_length) {
								std::unique_ptr<char[]> original_data(new char[data_length]);
								inputbuffer->lock();
								inputbuffer->remove(original_data.get(), data_length);
								restore.reset(new events::Streambuff);
								inputbuffer->unlock();

								//拷贝数据到缓冲区
								memcpy(total_original_data_.get() + total_postion_, original_data.get(), data_length);
								total_postion_ += data_length;
								if (total_postion_ >= filter_->max_reve_buf_size) {
									LOG_WARN << "接收缓冲超过最大可接收容量:" << filter_->max_reve_buf_size;
									total_postion_ = 0;
									return -1;
								}
								//判断是否是完整包
								if (ft != FrameType::Continuation) {
									if (ft == FrameType::Ping) {
										return send(nullptr, 0, FrameType::Pong);
									}
									else if (ft == FrameType::Pong) {
										return 0;
									}
									else if (ft == FrameType::Close) {
										return -2;
									}
									else {
										int ret = filter_->message_handle(original_data.get(), data_length);
										total_postion_ = 0;
										if (ret == -1) {
											return -1;
										}
									}
								}
							}
							else {
								LOG_WARN << "数据包数据不足,需要needlen:" << needlen << " 当前:" << inputbuffer->length();;
								inputbuffer->prependbuffer(restore);
								return 0;
							}
						}
						else {
							LOG_WARN << "数据包数据不足,需要needlen:" << needlen << " 当前:" << inputbuffer->length();;
							inputbuffer->prependbuffer(restore);
							return 0;
						}
					}
					else {
						LOG_WARN << "数据包数据不足,需要needlen:" << needlen << " 当前:" << inputbuffer->length();;
						inputbuffer->prependbuffer(restore);
						return 0;
					}
				}
			}
			return 0;
		}

		int Tcpip::send1(const void* data, size_t len, FrameType op) const
		{			const char* head = "\r\n";
			uint32_t datalen = htonl(static_cast<uint32_t>(len));
			size_t total_data_len = strlen(head) + sizeof(char) + sizeof(datalen) + len;
			std::unique_ptr<char[]> data_buffer(new char[total_data_len]);
			size_t pos = 0;
			memcpy(data_buffer.get(), head, strlen(head));
			pos += strlen(head);
			memcpy(data_buffer.get() + pos, &op, sizeof(char));
			pos += sizeof(char);
			memcpy(data_buffer.get() + pos, &datalen, sizeof(datalen));
			pos += sizeof(datalen);
			memcpy(data_buffer.get() + pos, data, len);
			pos += len;
			std::shared_ptr<events::EventBuffer> io = filter_->iobuf();
			if (io != nullptr) {
				return io->write(data_buffer.get(), total_data_len);
			}
			return -1;
		}

		int Tcpip::send(const void* data, size_t len, FrameType op) const {
			size_t numtimes = len / filter_->max_single_buf_size; //分多少个包
			size_t remaining = len % filter_->max_single_buf_size; //剩余数据

			int ret = 0;
			size_t pos = 0;
			for (size_t i = 0; i < numtimes; ++i)
			{
				ret = send1((const char*)data + pos, filter_->max_single_buf_size, FrameType::Continuation);
				if (ret == -1) {
					return ret;
				}
				pos += filter_->max_single_buf_size;
			}
			return send1((const char*)data + pos, remaining, op);
		}
		int Tcpip::send(std::string data, FrameType op) const {
			return send(data.c_str(), data.length(), op);
		}
	}
}
