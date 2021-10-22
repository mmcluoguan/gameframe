#ifndef SHYNET_NET_ACCEPTREACTORMGR_H
#define SHYNET_NET_ACCEPTREACTORMGR_H

#include "shynet/thread/acceptthread.h"
#include "shynet/utils/singleton.h"

namespace shynet {
namespace net {

    class AcceptReactorMgr final : public Nocopy {
        friend class utils::Singleton<AcceptReactorMgr>;
        AcceptReactorMgr();

    public:
        ~AcceptReactorMgr();

        void notify(const void* data, size_t len);

    private:
        std::shared_ptr<thread::AcceptThread> find_rtk() const;
    };
}
}

#endif
