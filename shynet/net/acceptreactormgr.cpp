#include "shynet/net/acceptreactormgr.h"
#include "shynet/pool/threadpool.h"
#include "shynet/utils/logger.h"

namespace shynet {
namespace net {
    AcceptReactorMgr::AcceptReactorMgr()
    {
    }

    AcceptReactorMgr::~AcceptReactorMgr()
    {
    }

    void AcceptReactorMgr::notify(const void* data, size_t len)
    {
        std::shared_ptr<thread::AcceptThread> rtk = find_rtk();
        if (rtk != nullptr) {
            rtk->notify(data, len);
        } else
            LOG_WARN << "没有可用的 AcceptThread";
    }

    std::shared_ptr<thread::AcceptThread> AcceptReactorMgr::find_rtk() const
    {
        std::shared_ptr<thread::AcceptThread> ath;
        int tot = -1;
        utils::Singleton<pool::ThreadPool>::get_instance()
            .foreach_acceptThs([&](std::weak_ptr<thread::AcceptThread> it) {
                std::shared_ptr<thread::AcceptThread> rtkp = it.lock();
                if (rtkp != nullptr) {
                    if (tot == -1) {
                        tot = rtkp->event_tot();
                        ath = rtkp;
                    } else {
                        if (rtkp->event_tot() < tot) {
                            tot = rtkp->event_tot();
                            ath = rtkp;
                        }
                    }
                }
            });
        return ath;
    }
}
}
