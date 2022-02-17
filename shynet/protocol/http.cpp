#include "shynet/protocol/http.h"
#include "3rd/jemalloc/jemalloc.h"
#include "shynet/pool/threadpool.h"
#include "shynet/protocol/filterproces.h"
#include "shynet/task/acceptreadiotask.h"
#include "shynet/utils/logger.h"
#include <cstring>

namespace shynet {
namespace protocol {
    Http::Http(FilterProces* filter)
        : filter_(filter)
    {
    }

    net::InputResult Http::process(std::function<void(std::unique_ptr<char[]>, size_t)> cb)
    {
        std::shared_ptr<events::Streambuff> inputbuffer = filter_->iobuf()->inputbuffer();
        if (inputbuffer->length() >= filter_->max_reve_buf_size) {
            LOG_WARN << "接收的数据超过最大可接收容量:" << filter_->max_reve_buf_size;
            return net::InputResult::INITIATIVE_CLOSE;
        }
        std::shared_ptr<events::Streambuff> restore = std::make_shared<events::Streambuff>();
        while (inputbuffer->length() > 0) {
            if (filter_->ident() == FilterProces::Identity::ACCEPTOR) {
                net::InputResult ret = process_requset(inputbuffer, restore, cb);
                if (ret != net::InputResult::SUCCESS)
                    return ret;
            } else if (filter_->ident() == FilterProces::Identity::CONNECTOR) {
                net::InputResult ret = process_responses(inputbuffer, restore, cb);
                if (ret != net::InputResult::SUCCESS)
                    return ret;
            }
        }
        return net::InputResult::SUCCESS;
    }

    net::InputResult Http::process_requset(std::shared_ptr<events::Streambuff> inputbuffer,
        std::shared_ptr<events::Streambuff> restore,
        std::function<void(std::unique_ptr<char[]>, size_t)> cb)
    {
        if (requset_.step() == Step::UNINIT) {
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
                std::string strline(line, len);
                je_free(line);
                int ret = requset_.requset_uninit(strline);
                if (ret == -1) {
                    LOG_WARN << "协议错误";
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
            if (line == nullptr) {
                je_free(line);
                LOG_WARN << "协议错误";
                return net::InputResult::INITIATIVE_CLOSE;
            } else {
                std::string strline(line, len);
                je_free(line);
                int ret = requset_.requset_init(strline);
                if (ret == -1) {
                    return net::InputResult::INITIATIVE_CLOSE;
                }
            }
        } else if (requset_.step() == Step::Parse) {
            size_t data_length = requset_.data_length();
            if (data_length == 0) {
                LOG_WARN << "没有POST数据";
            } else {
                if (inputbuffer->length() >= data_length) {
                    std::unique_ptr<char[]> original_data(new char[data_length]);
                    inputbuffer->lock();
                    inputbuffer->remove(original_data.get(), data_length);
                    restore.reset(new events::Streambuff);
                    inputbuffer->unlock();
                    cb(std::move(original_data), data_length);
                    requset_.set_step(Step::UNINIT);
                } else {
                    LOG_TRACE << "数据包数据不足,需要data_length:" << data_length << " 当前:" << inputbuffer->length();
                    inputbuffer->prependbuffer(restore);
                    return net::InputResult::DATA_INCOMPLETE;
                }
            }
        }
        return net::InputResult::SUCCESS;
    }

    net::InputResult Http::process_responses(std::shared_ptr<events::Streambuff> inputbuffer,
        std::shared_ptr<events::Streambuff> restore,
        std::function<void(std::unique_ptr<char[]>, size_t)> cb)
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
            std::string strline(line, len);
            je_free(line);
            int ret = responses_.responses_init(strline);
            if (ret == -1) {
                return net::InputResult::INITIATIVE_CLOSE;
            }
        } else if (responses_.step() == Step::Parse) {
            size_t data_length = responses_.data_length();
            if (data_length == 0) {
                LOG_WARN << "没有POST数据";
            } else {
                if (inputbuffer->length() >= data_length) {
                    std::unique_ptr<char[]> original_data(new char[data_length]);
                    inputbuffer->lock();
                    inputbuffer->remove(original_data.get(), data_length);
                    restore.reset(new events::Streambuff);
                    inputbuffer->unlock();
                    cb(std::move(original_data), data_length);
                    responses_.set_step(Step::UNINIT);
                } else {
                    LOG_TRACE << "数据包数据不足,需要data_length:" << data_length << " 当前:" << inputbuffer->length();
                    inputbuffer->prependbuffer(restore);
                    return net::InputResult::DATA_INCOMPLETE;
                }
            }
        }
        return net::InputResult::DATA_INCOMPLETE;
    }

    int Http::send_responses(const void* data, size_t len,
        std::string server_name,
        std::string content_type,
        std::map<std::string, std::string>* server_heads,
        int status_code,
        std::string status_desc,
        std::string version) const
    {
        std::stringstream stream;
        stream << version << " " << status_code << " " << status_desc << "\r\n";
        stream << "Server:" << server_name << "\r\n";
        stream << "Content-Type:" << content_type << "\r\n";
        stream << "Content-Length:" << len << "\r\n";
        if (server_heads != nullptr) {
            for (auto&& [key, value] : *server_heads) {
                if (value.empty())
                    continue;
                stream << key << ":" << value << "\r\n";
            }
        }
        stream << "\r\n";
        size_t total_data_len = stream.str().length() + len;
        std::unique_ptr<char[]> data_buffer(new char[total_data_len]);
        size_t pos = 0;
        memcpy(data_buffer.get(), stream.str().c_str(), stream.str().length());
        pos += stream.str().length();
        memcpy(data_buffer.get() + pos, data, len);
        std::shared_ptr<events::EventBuffer> io = filter_->iobuf();
        if (io != nullptr) {
            return io->write(data_buffer.get(), total_data_len);
        }
        return -1;
    }

    int Http::send_responses(const std::string& data,
        std::string server_name,
        std::string content_type,
        std::map<std::string, std::string>* server_heads,
        int status_code,
        std::string status_desc,
        std::string version) const
    {
        return send_responses(data.c_str(), data.length(), server_name, content_type, server_heads, status_code, status_desc, version);
    }

    int Http::send_requset(const void* data, size_t len,
        std::string host,
        std::string path,
        std::string version) const
    {
        std::stringstream stream;
        std::string method = "POST";
        if (data == nullptr)
            method = "GET";
        stream << method << " " << path << " " << version << "\r\n";
        stream << "Host:" << host << "\r\n";
        stream << "Content-Length:" << len << "\r\n";
        stream << "Connection:close\r\n";
        stream << "\r\n";
        size_t total_data_len = stream.str().length();
        if (data != nullptr) {
            total_data_len += len;
            std::unique_ptr<char[]> data_buffer(new char[total_data_len]);
            size_t pos = 0;
            memcpy(data_buffer.get(), stream.str().c_str(), stream.str().length());
            pos += stream.str().length();
            memcpy(data_buffer.get() + pos, data, len);
            std::shared_ptr<events::EventBuffer> io = filter_->iobuf();
            if (io != nullptr) {
                return io->write(data_buffer.get(), total_data_len);
            }
            return -1;
        }
        std::shared_ptr<events::EventBuffer> io = filter_->iobuf();
        if (io != nullptr) {
            return io->write(stream.str().c_str(), total_data_len);
        }
        return -1;
    }
    int Http::send_requset(const std::string& data, std::string host, std::string path, std::string version) const
    {
        return send_requset(data.c_str(), data.length(), host, path, version);
    }
}
}
