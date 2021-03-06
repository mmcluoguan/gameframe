#include "shynet/protocol/websocket.h"
#include "3rd/jemalloc/jemalloc.h"
#include "shynet/crypto/base64.h"
#include "shynet/crypto/sha1.h"
#include "shynet/pool/threadpool.h"
#include "shynet/protocol/filterproces.h"
#include "shynet/utils/logger.h"
#include "shynet/utils/stuff.h"
#include <cstring>

namespace shynet {
namespace protocol {
    WebSocket::WebSocket(FilterProces* filter)
        : filter_(filter)
        , total_original_data_(std::make_shared<events::Streambuff>())
    {
    }

    net::InputResult WebSocket::process(std::function<void(std::unique_ptr<char[]>, size_t)> cb)
    {
        std::shared_ptr<events::Streambuff> inputbuffer = filter_->iobuf()->inputbuffer();
        std::shared_ptr<events::Streambuff> restore = std::make_shared<events::Streambuff>();
        while (inputbuffer->length() > 0) {
            if (filter_->ident() == FilterProces::Identity::ACCEPTOR) {
                if (status_ == Status::Unconnect) {
                    net::InputResult ret = process_requset(inputbuffer, restore);
                    if (ret != net::InputResult::SUCCESS)
                        return ret;
                } else if (status_ == Status::Handsharked) {
                    net::InputResult ret = process_data(inputbuffer, restore, cb);
                    if (ret != net::InputResult::SUCCESS)
                        return ret;
                }

            } else if (filter_->ident() == FilterProces::Identity::CONNECTOR) {
                if (status_ == Status::Unconnect) {
                    net::InputResult ret = process_responses(inputbuffer, restore);
                    if (ret != net::InputResult::SUCCESS)
                        return ret;
                } else if (status_ == Status::Handsharked) {
                    net::InputResult ret = process_data(inputbuffer, restore, cb);
                    if (ret != net::InputResult::SUCCESS)
                        return ret;
                }
            }
        }
        return net::InputResult::SUCCESS;
    }

    net::InputResult WebSocket::process_requset(std::shared_ptr<events::Streambuff> inputbuffer,
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
                LOG_WARN << "????????????";
                return net::InputResult::INITIATIVE_CLOSE;
            } else {
                std::string strline(line, len);
                je_free(line);
                int ret = requset_.requset_uninit(strline);
                if (ret == -1) {
                    return net::InputResult::INITIATIVE_CLOSE;
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
                return net::InputResult::INITIATIVE_CLOSE;
            } else {
                const char* key;
                if (requset_.step() == Step::Parse && (key = requset_.websocket_key()) != nullptr) {
                    ret = upgrade(key);
                    if (ret == -1) {
                        return net::InputResult::INITIATIVE_CLOSE;
                    }
                }
            }
        }
        return net::InputResult::SUCCESS;
    }

    net::InputResult WebSocket::process_responses(std::shared_ptr<events::Streambuff> inputbuffer,
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
                LOG_WARN << "????????????";
                return net::InputResult::INITIATIVE_CLOSE;
            } else {
                std::string strline(line, len);
                je_free(line);
                int ret = responses_.responses_uninit(strline);
                if (ret == -1) {
                    return net::InputResult::INITIATIVE_CLOSE;
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
                LOG_WARN << "????????????";
                return net::InputResult::INITIATIVE_CLOSE;
            } else {
                std::string strline(line, len);
                je_free(line);
                int ret = responses_.responses_init(strline);
                if (ret == -1) {
                    return net::InputResult::INITIATIVE_CLOSE;
                } else {
                    const char* key;
                    if (responses_.step() == Step::Parse && (key = responses_.websocket_key()) != nullptr) {
                        ret = verify(key);
                        if (ret == -1) {
                            return net::InputResult::INITIATIVE_CLOSE;
                        }
                    }
                }
            }
        }
        return net::InputResult::SUCCESS;
    }

    net::InputResult WebSocket::process_data(std::shared_ptr<events::Streambuff> inputbuffer,
        std::shared_ptr<events::Streambuff> restore,
        std::function<void(std::unique_ptr<char[]>, size_t)> cb)
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
                    LOG_TRACE << "?????????????????????,??????needlen:" << needlen;
                    inputbuffer->lock();
                    inputbuffer->prependbuffer(restore);
                    inputbuffer->unlock();
                    return net::InputResult::DATA_INCOMPLETE;
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
                    LOG_TRACE << "?????????????????????,??????needlen:" << needlen;
                    inputbuffer->lock();
                    inputbuffer->prependbuffer(restore);
                    inputbuffer->unlock();
                    return net::InputResult::DATA_INCOMPLETE;
                }
            }
        } else {
            LOG_TRACE << "?????????????????????,??????needlen:" << needlen;
            inputbuffer->lock();
            inputbuffer->prependbuffer(restore);
            inputbuffer->unlock();
            return net::InputResult::DATA_INCOMPLETE;
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
                LOG_TRACE << "?????????????????????,??????needlen:" << needlen;
                inputbuffer->lock();
                inputbuffer->prependbuffer(restore);
                inputbuffer->unlock();
                return net::InputResult::DATA_INCOMPLETE;
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
                //??????Ping?????????
                char timesbuf[sizeof(uint64_t) * 2] = { 0 };
                uint64_t ser_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                ser_timestamp = utils::stuff::hl64ton(ser_timestamp);
                memcpy(timesbuf, &ser_timestamp, sizeof(ser_timestamp));
                uint64_t ping_timestamp;
                memcpy(&ping_timestamp, original_data.get(), sizeof(ping_timestamp));
                memcpy(timesbuf + sizeof(ser_timestamp), &ping_timestamp, sizeof(ping_timestamp));
                send(&timesbuf, sizeof(timesbuf), FrameType::Pong);
                return net::InputResult::SUCCESS;
            } else if (ft == FrameType::Pong) {
                //????????????
                uint64_t ser_timestamp;
                memcpy(&ser_timestamp, original_data.get(), sizeof(ser_timestamp));
                ser_timestamp = utils::stuff::ntohl64(ser_timestamp);
                uint64_t ping_timestamp;
                memcpy(&ping_timestamp, original_data.get() + sizeof(ser_timestamp), sizeof(ping_timestamp));
                ping_timestamp = utils::stuff::ntohl64(ping_timestamp);
                uint64_t delay
                    = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() - ping_timestamp;
                uint64_t remote_timestamp = ser_timestamp + delay / 2;
                LOG_TRACE << "delay:" << delay << "(ms) remote_timestamp:" << remote_timestamp << "(ms)";
                filter_->set_late_delay(delay);
                filter_->set_remote_exact_timestamp(remote_timestamp);
                return net::InputResult::SUCCESS;
            } else if (ft == FrameType::Close) {
                return net::InputResult::PASSIVE_CLOSE;
            } else {
                //????????????????????????
                if (fin == 1) {
                    //???????????????
                    size_t total_length = total_original_data_->length();
                    if (total_length > 0) {
                        std::unique_ptr<char[]> complete_data(new char[total_length]);
                        total_original_data_->remove(complete_data.get(), total_length);
                        cb(std::move(complete_data), total_length);
                    } else {
                        cb(std::move(original_data), data_length);
                    }
                } else {
                    //??????????????????,????????????????????????
                    total_original_data_->add(original_data.get(), data_length);
                    size_t total_length = total_original_data_->length();
                    if (total_length >= filter_->max_reve_buf_size) {
                        LOG_WARN << "???????????????????????????????????????:" << filter_->max_reve_buf_size;
                        total_original_data_->drain(total_length);
                        return net::InputResult::INITIATIVE_CLOSE;
                    }
                }
            }
        } else {
            LOG_TRACE << "?????????????????????,??????data_length:" << data_length << " ??????:" << buffer_length << " fin:" << fin;
            inputbuffer->lock();
            inputbuffer->prependbuffer(restore);
            inputbuffer->unlock();
            //usleep(1 * 1000 * 1000);//??????????????????1??? = 1000?????? ???1?????? = 1000??????
            return net::InputResult::DATA_INCOMPLETE;
        }
        return net::InputResult::SUCCESS;
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
            LOG_WARN << "websoket????????????????????????";
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

        size_t numtimes = len / filter_->max_single_buf_size; //???????????????
        size_t remaining = len % filter_->max_single_buf_size; //????????????

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

    int WebSocket::send(const std::string& data, FrameType op) const
    {
        return send(data.c_str(), data.length(), op);
    }

}
}
