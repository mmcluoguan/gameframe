#ifndef SHYNET_NET_CONNECTREACTORMGR_H
#define SHYNET_NET_CONNECTREACTORMGR_H

#include "shynet/net/connectevent.h"
#include "shynet/utils/singleton.h"
#include <unordered_map>

namespace shynet {
namespace net {
    class ConnectReactorMgr final : public Nocopy {
        friend class utils::Singleton<ConnectReactorMgr>;
        ConnectReactorMgr();

    public:
        ~ConnectReactorMgr();

        int add(std::shared_ptr<ConnectEvent> v);
        bool remove(int k);
        std::shared_ptr<ConnectEvent> find(int k);
        std::shared_ptr<ConnectEvent> find(const std::string ip, unsigned short port);

    private:
        void notify(const void* data, size_t len);

    private:
        std::mutex cnt_mutex_;
        std::unordered_map<int, std::shared_ptr<ConnectEvent>> cnts_;
    };
}
}

#endif
