#include "shynet/protocol/websocket.h"
#include "shynet/3rd/jemalloc/jemalloc.h"
#include "shynet/crypto/base64.h"
#include "shynet/crypto/sha1.h"
#include "shynet/protocol/filterproces.h"
#include "shynet/utils/logger.h"
#include "shynet/utils/stuff.h"
#include <cstring>

namespace shynet {
namespace protocol {
    WebSocket::WebSocket(FilterProces* filter)
        : filter_(filter)
        , total_original_data_(new char[filter_->max_reve_buf_size])
    {
    }
    WebSocket::~WebSocket()
    {
    }

    int WebSocket::process()
    {
        std::shared_ptr<events::Streambuff> inputbuffer = filter_->iobuf()->inputbuffer();
        std::shared_ptr<events::Streambuff> restore = std::make_shared<events::Streambuff>();
        while (inputbuffer->length() > 0) {
            if (filter_->ident() == FilterProces::Identity::ACCEPTOR) {
                if (status_ == Status::Unconnect) {
                    int ret = process_requset(inputbuffer, restore);
                    if (ret < 0) {
                        return ret;
                    }
                } else if (status_ == Status::Handsharked) {
                    int ret = process_data(inputbuffer, restore);
                    if (ret < 0) {
                        return ret;
                    }
                }

            } else if (filter_->ident() == FilterProces::Identity::CONNECTOR) {
                if (status_ == Status::Unconnect) {
                    int ret = process_responses(inputbuffer, restore);
                    if (ret < 0) {
                        return ret;
                    }
                } else if (status_ == Status::Handsharked) {
                    int ret = process_data(inputbuffer, restore);
                    if (ret < 0) {
                        return ret;
                    }
                }
            }
        }
        return 0;
    }

    int WebSocket::process_requset(std::shared_ptr<events::Streambuff> inputbuffer,
        std::shared_ptr<events::Streambuff> restore)
    {
        if (requset_.step() == Step::UNINIT) {
            size_t len;
            inputbuffer->lock();
            char* line = inputbuffer->readln(&len, evbuffer_eol_style::EVBUFFER_EOL_CRLF_STRICT);
            restore->add(line, len);
            restore->addprintf("\r\n");
            inputbuffer->unlock();
            if (line == nullptr) {
                LOG_WARN << "协议错误";
                return -1;
            } else {
                std::string strline(line, len);
                je_free(line);
                int ret = requset_.requset_uninit(strline);
                if (ret == -1) {
                    return -1;
                }
            }
        } else if (requset_.step() == Step::INIT) {
            size_t len;
            inputbuffer->lock();
            char* line = inputbuffer->readln(&len, evbuffer_eol_style::EVBUFFER_EOL_CRLF_STRICT);
            restore->add(line, len);
            restore->addprintf("\r\n");
            inputbuffer->unlock();
            std::string strline(line, len);
            je_free(line);
            int ret = requset_.requset_init(strline);
            if (ret == -1) {
                return -1;
            } else {
                const char* key;
                if (requset_.step() == Step::Parse && (key = requset_.websocket_key()) != nullptr) {
                    ret = upgrade(key);
                    if (ret == -1) {
                        return -1;
                    }
                }
            }
        }
        return 0;
    }

    int WebSocket::process_responses(std::shared_ptr<events::Streambuff> inputbuffer,
        std::shared_ptr<events::Streambuff> restore)
    {
        if (responses_.step() == Step::UNINIT) {
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
            } else {
                std::string strline(line, len);
                je_free(line);
                int ret = responses_.responses_uninit(strline);
                if (ret == -1) {
                    return -1;
                }
            }
        } else if (responses_.step() == Step::INIT) {
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
            } else {
                std::string strline(line, len);
                je_free(line);
                int ret = responses_.responses_init(strline);
                if (ret == -1) {
                    return -1;
                } else {
                    const char* key;
                    if (responses_.step() == Step::Parse && (key = responses_.websocket_key()) != nullptr) {
                        ret = verify(key);
                        if (ret == -1) {
                            return -1;
                        }
                    }
                }
            }
        }
        return 0;
    }

    int WebSocket::process_data(std::shared_ptr<events::Streambuff> inputbuffer,
        std::shared_ptr<events::Streambuff> restore)
    {
        size_t n = 0, mask_len = 0;
        char buf[2];
        size_t needlen = sizeof(buf);
        size_t data_length;
        FrameType ft;
        int fin = 0;
        if (inputbuffer->length() >= needlen) {
            inputbuffer->lock();
            inputbuffer->remove(buf, needlen);
            restore->add(buf, needlen);
            inputbuffer->unlock();
            fin = (unsigned char)buf[0] >> 7;
            ft = (FrameType)(buf[0] & 0x0f);
            n = buf[1] & 0x7f;
            mask_len = buf[1] & 128 ? 4 : 0;
            if (n < 126) {
                data_length = n;
            } else if (n == 126) {
                needlen = sizeof(uint16_t);
                if (inputbuffer->length() >= needlen) {
                    inputbuffer->lock();
                    inputbuffer->remove(&data_length, needlen);
                    restore->add(&data_length, needlen);
                    inputbuffer->unlock();
                    data_length = ntohs((uint16_t)data_length);
                } else {
                    LOG_WARN << "数据包数据不足,需要needlen:" << needlen;
                    inputbuffer->lock();
                    inputbuffer->prependbuffer(restore);
                    inputbuffer->unlock();
                    return 0;
                }
            } else if (n > 126) {
                needlen = sizeof(uint64_t);
                if (inputbuffer->length() >= needlen) {
                    inputbuffer->lock();
                    inputbuffer->remove(&data_length, needlen);
                    restore->add(&data_length, needlen);
                    inputbuffer->unlock();
                    data_length = utils::stuff::ntohl64(data_length);
                } else {
                    LOG_WARN << "数据包数据不足,需要needlen:" << needlen;
                    inputbuffer->lock();
                    inputbuffer->prependbuffer(restore);
                    inputbuffer->unlock();
                    return 0;
                }
            }
        } else {
            LOG_WARN << "数据包数据不足,需要needlen:" << needlen;
            inputbuffer->lock();
            inputbuffer->prependbuffer(restore);
            inputbuffer->unlock();
            return 0;
        }
        char mask[4];
        if (mask_len > 0) {
            needlen = mask_len;
            if (inputbuffer->length() >= needlen) {
                inputbuffer->lock();
                inputbuffer->remove(mask, needlen);
                restore->add(mask, needlen);
                inputbuffer->unlock();
            } else {
                LOG_WARN << "数据包数据不足,需要needlen:" << needlen;
                inputbuffer->lock();
                inputbuffer->prependbuffer(restore);
                inputbuffer->unlock();
                return 0;
            }
        }
        size_t buffer_length = inputbuffer->length();
        if (buffer_length >= data_length) {
            std::unique_ptr<char[]> original_data(new char[data_length]);
            inputbuffer->lock();
            inputbuffer->remove(original_data.get(), data_length);
            restore.reset(new events::Streambuff);
            inputbuffer->unlock();
            if (mask_len > 0) {
                for (size_t i = 0; i < data_length; i++) {
                    original_data.get()[i] ^= mask[i % 4];
                }
            }
            if (ft == FrameType::Ping) {
                return send(nullptr, 0, FrameType::Pong);
            } else if (ft == FrameType::Pong) {
                return 0;
            } else if (ft == FrameType::Close) {
                return -2;
            } else {
                //拷贝数据到缓冲区
                memcpy(total_original_data_.get() + total_postion_, original_data.get(), data_length);
                total_postion_ += data_length;
                if (total_postion_ >= filter_->max_reve_buf_size) {
                    LOG_WARN << "接收缓冲超过最大可接收容量:" << filter_->max_reve_buf_size;
                    total_postion_ = 0;
                    return -1;
                }
                //判断是否是完整包
                if (fin == 1) {
                    int ret = filter_->message_handle(total_original_data_.get(), total_postion_);
                    total_postion_ = 0;
                    if (ret == -1 || ft == FrameType::Close) {
                        return -1;
                    }
                }
            }
        } else {
            LOG_WARN << "数据包数据不足,需要data_length:" << data_length << " 当前:" << buffer_length << " fin:" << fin;
            inputbuffer->lock();
            inputbuffer->prependbuffer(restore);
            inputbuffer->unlock();
            //usleep(1 * 1000 * 1000);//单位是微秒，1秒 = 1000毫秒 ，1毫秒 = 1000微秒
            return -3;
        }
        return 0;
    }

    int WebSocket::upgrade(const char* key)
    {

        std::string buf = "HTTP/1.1 101 Switching Protocols\r\nUpgrade:websocket\r\nConnection:Upgrade\r\nSec-WebSocket-Accept:";
        std::string newkey(key);
        newkey += magic_key_;
        unsigned char md[20];
        crypto::sha1::calc(newkey.c_str(), newkey.length(), md);
        newkey = crypto::base64_encode(md, 20);
        buf += newkey += "\r\n\r\n";
        status_ = Status::Handsharked;
        return filter_->iobuf()->write(buf.c_str(), buf.length());
    }

    int WebSocket::verify(const char* key)
    {
        if (strcmp(request_key_.c_str(), key) == 0) {
            status_ = Status::Handsharked;
            filter_->complete();
            return 0;
        } else {
            LOG_WARN << "websoket与服务器握手失败";
            return -1;
        }
    }

    int WebSocket::request_handshake()
    {
        std::string buf = "GET / HTTP/1.1\r\nUpgrade:websocket\r\nConnection:Upgrade\r\nSec-WebSocket-Version:13\r\nSec-WebSocket-Key:";
        unsigned char mask[16];
        utils::stuff::random(mask, sizeof(mask));
        std::string newkey = crypto::base64_encode(mask, sizeof(mask));
        buf += newkey + "\r\n\r\n";
        unsigned char md[20];
        newkey += magic_key_;
        crypto::sha1::calc(newkey.c_str(), newkey.length(), md);
        request_key_ = crypto::base64_encode(md, 20);
        return filter_->iobuf()->write(buf.c_str(), buf.length());
    }

    int WebSocket::send1(const void* data, size_t len, FrameType op) const
    {
        unsigned char header[10];
        size_t header_len = 0;
        header[0] = (uint8_t)((int)op | 128);
        if (op == FrameType::Continuation) {
            header[0] = 0;
        }
        bool ismask = filter_->ident() == FilterProces::Identity::CONNECTOR ? true : false;
        if (len < 126) {
            header[1] = (unsigned char)((len & 0xff) | (ismask ? 128 : 0));
            header_len = 2;
        } else if (len < 65536) {
            header[1] = 126 | (ismask ? 128 : 0);
            uint16_t tmp = htons((uint16_t)len);
            memcpy(&header[2], &tmp, sizeof(tmp));
            header_len = 4;
        } else {
            header[1] = 127 | (ismask ? 128 : 0);
            uint64_t nlen = utils::stuff::hl64ton(len);
            memcpy(&header[2], &nlen, sizeof(nlen));
            header_len = 10;
        }

        std::shared_ptr<events::EventBuffer> io = filter_->iobuf();
        if (io != nullptr) {
            if (ismask) {
                unsigned char mask[4];
                size_t total_data_len = header_len + sizeof(mask) + len;
                std::unique_ptr<char[]> data_buffer(new char[total_data_len]);

                size_t pos = 0;
                memcpy(data_buffer.get() + pos, header, header_len);
                pos += header_len;

                utils::stuff::random(mask, sizeof(mask));
                memcpy(data_buffer.get() + pos, mask, sizeof(mask));
                pos += sizeof(mask);

                memcpy(data_buffer.get() + pos, data, len);

                size_t index = 0;
                for (char* i = data_buffer.get() + pos; i < data_buffer.get() + pos + len; i++) {
                    *i ^= static_cast<char>(mask[index % 4]);
                    index++;
                }
                io->write(data_buffer.get(), total_data_len);
            } else {
                io->write(header, header_len);
                io->write(data, len);
            }
            return 0;
        }
        return -1;
    }

    int WebSocket::send(const void* data, size_t len, FrameType op) const
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

    int WebSocket::send(std::string data, FrameType op) const
    {
        return send(data.c_str(), data.length(), op);
    }

}
}
