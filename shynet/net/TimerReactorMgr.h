#ifndef SHYNET_NET_TIMERREACTORMGR_H
#define SHYNET_NET_TIMERREACTORMGR_H

#include "shynet/net/TimerEvent.h"
#include "shynet/utils/Singleton.h"
#include <unordered_map>

namespace shynet {
namespace net {
    class TimerReactorMgr final : public Nocopy {
        friend class utils::Singleton<TimerReactorMgr>;
        TimerReactorMgr();

    public:
        ~TimerReactorMgr();

        int add(std::shared_ptr<TimerEvent> v);
        bool remove(int k);
        std::shared_ptr<TimerEvent> find(int k);

    private:
        void notify(const void* data, size_t len);

    private:
        std::mutex times_mutex_;
        std::unordered_map<int, std::shared_ptr<TimerEvent>> times_;
    };
} // namespace net
} // namespace shynet

#endif
