#include "shynet/protocol/tcpip.h"
#include "3rd/jemalloc/jemalloc.h"
#include "shynet/pool/threadpool.h"
#include "shynet/protocol/filterproces.h"
#include "shynet/task/acceptreadiotask.h"
#include "shynet/utils/logger.h"
#include <cstring>

namespace shynet {
namespace protocol {
    Tcpip::Tcpip(FilterProces* filter)
        : filter_(filter)
        , total_original_data_(std::make_shared<events::Streambuff>())
    {
    }

    net::InputResult Tcpip::process(std::function<void(std::unique_ptr<char[]>, size_t)> cb)
    {
        std::shared_ptr<events::Streambuff> inputbuffer = filter_->iobuf()->inputbuffer();
        std::shared_ptr<events::Streambuff> restore = std::make_shared<events::Streambuff>();
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
                return net::InputResult::INITIATIVE_CLOSE;
            } else {
                je_free(line);
                FrameType ft;
                size_t needlen = sizeof(char);
                if (inputbuffer->length() >= needlen) {
                    //读取帧类型
                    inputbuffer->lock();
                    char cft;
                    inputbuffer->remove(&cft, sizeof(char));
                    ft = static_cast<FrameType>(cft);
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
                            //判断是否是完整包
                            if (ft != FrameType::Continuation) {
                                //数据包完整
                                if (ft == FrameType::Ping) {
                                    //读取Ping时间戳
                                    char timesbuf[sizeof(uint64_t) * 2] = { 0 };
                                    uint64_t ser_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                                    memcpy(timesbuf, &ser_timestamp, sizeof(ser_timestamp));
                                    uint64_t ping_timestamp;
                                    inputbuffer->remove(&ping_timestamp, sizeof(data_length));
                                    memcpy(timesbuf + sizeof(ser_timestamp), &ping_timestamp, sizeof(ping_timestamp));
                                    send(&timesbuf, sizeof(timesbuf), FrameType::Pong);
                                } else if (ft == FrameType::Pong) {
                                    //计算延迟
                                    uint64_t ser_timestamp;
                                    inputbuffer->remove(&ser_timestamp, sizeof(ser_timestamp));
                                    uint64_t ping_timestamp;
                                    inputbuffer->remove(&ping_timestamp, sizeof(ping_timestamp));
                                    uint64_t delay
                                        = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() - ping_timestamp;
                                    uint64_t remote_timestamp = ser_timestamp + delay / 2;
                                    LOG_DEBUG << "delay:" << delay << "(ms) remote_timestamp:" << remote_timestamp << "(ms)";
                                    filter_->set_late_delay(delay);
                                    filter_->set_remote_exact_timestamp(remote_timestamp);
                                } else if (ft == FrameType::Close) {
                                    return net::InputResult::PASSIVE_CLOSE;
                                } else {
                                    size_t total_length = total_original_data_->length();
                                    std::shared_ptr<task::AcceptReadIoTask> io;
                                    if (total_length > 0) {
                                        std::unique_ptr<char[]> complete_data(new char[total_length]);
                                        total_original_data_->remove(complete_data.get(), total_length);
                                        cb(std::move(complete_data), total_length);
                                    } else {
                                        std::unique_ptr<char[]> original_data(new char[data_length]);
                                        inputbuffer->lock();
                                        inputbuffer->remove(original_data.get(), data_length);
                                        restore.reset(new events::Streambuff);
                                        inputbuffer->unlock();
                                        cb(std::move(original_data), data_length);
                                    }
                                }
                            } else {
                                //数据包不完整,拷贝数据到缓冲区
                                inputbuffer->lock();
                                inputbuffer->removebuffer(total_original_data_, data_length);
                                restore.reset(new events::Streambuff);
                                inputbuffer->unlock();
                                size_t total_length = total_original_data_->length();
                                if (total_length >= filter_->max_reve_buf_size) {
                                    LOG_WARN << "接收缓冲超过最大可接收容量:" << filter_->max_reve_buf_size;
                                    total_original_data_->drain(total_length);
                                    return net::InputResult::INITIATIVE_CLOSE;
                                }
                            }
                        } else {
                            LOG_TRACE << "数据包数据不足,需要needlen:" << needlen << " 当前:" << inputbuffer->length();
                            ;
                            inputbuffer->prependbuffer(restore);
                            return net::InputResult::DATA_INCOMPLETE;
                        }
                    } else {
                        LOG_TRACE << "数据包数据不足,需要needlen:" << needlen << " 当前:" << inputbuffer->length();
                        ;
                        inputbuffer->prependbuffer(restore);
                        return net::InputResult::DATA_INCOMPLETE;
                    }
                } else {
                    LOG_TRACE << "数据包数据不足,需要needlen:" << needlen << " 当前:" << inputbuffer->length();
                    ;
                    inputbuffer->prependbuffer(restore);
                    return net::InputResult::DATA_INCOMPLETE;
                }
            }
        }
        return net::InputResult::SUCCESS;
    }

    int Tcpip::send1(const void* data, size_t len, FrameType op) const
    {
        const char* head = "\r\n";
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

    int Tcpip::send(const void* data, size_t len, FrameType op) const
    {
        size_t numtimes = len / filter_->max_single_buf_size; //分多少个包
        size_t remaining = len % filter_->max_single_buf_size; //剩余数据

        int ret = 0;
        size_t pos = 0;
        for (size_t i = 0; i < numtimes; ++i) {
            ret = send1((const char*)data + pos, filter_->max_single_buf_size, FrameType::Continuation);
            if (ret == -1) {
                return ret;
            }
            pos += filter_->max_single_buf_size;
        }
        return send1((const char*)data + pos, remaining, op);
    }
    int Tcpip::send(const std::string& data, FrameType op) const
    {
        return send(data.c_str(), data.length(), op);
    }
}
}
