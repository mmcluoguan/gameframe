#ifndef SHYNET_PROTOCOL_TCPIP_H
#define SHYNET_PROTOCOL_TCPIP_H

#include "shynet/basic.h"
#include "shynet/events/streambuff.h"

namespace shynet {
namespace protocol {
    class FilterProces;

    class Tcpip : public Nocopy {
    public:
        enum class FrameType {
            Continuation,
            Binary,
            Close = 8,
            Ping = 9,
            Pong = 10,
        };

        explicit Tcpip(FilterProces* filter);
        ~Tcpip();

        int process();
        int send1(const void* data, size_t len, FrameType op) const;
        int send(const void* data, size_t len, FrameType op) const;
        int send(std::string data, FrameType op) const;

    private:
        FilterProces* filter_;
        //最大接收缓冲大小
        std::unique_ptr<char[]> total_original_data_;
        size_t total_postion_ = 0;
    };
}
}

#endif
