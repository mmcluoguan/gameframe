#include "shynet/net/listenreactormgr.h"
#include "shynet/utils/logger.h"

namespace shynet {
namespace net {

    ListenReactorMgr::ListenReactorMgr()
    {
    }

    ListenReactorMgr::~ListenReactorMgr()
    {
    }

    void ListenReactorMgr::notify(const void* data, size_t len)
    {
        std::shared_ptr<thread::ListenThread> lth = utils::Singleton<pool::ThreadPool>::get_instance().listernTh().lock();
        if (lth != nullptr) {
            lth->notify(data, len);
        } else
            LOG_WARN << "没有可用的 ListenThread";
    }

    int ListenReactorMgr::add(std::shared_ptr<ListenEvent> v)
    {
        static int serverid = 0;
        {
            std::lock_guard<std::mutex> lock(les_mutex_);
            serverid++;
            les_.insert({ serverid, v });
            v->set_serverid(serverid);
        }
        notify(&serverid, sizeof(serverid));
        return serverid;
    }

    bool ListenReactorMgr::remove(int k)
    {
        std::lock_guard<std::mutex> lock(les_mutex_);
        return les_.erase(k) > 0 ? true : false;
    }

    std::shared_ptr<ListenEvent> ListenReactorMgr::find(int k)
    {
        std::lock_guard<std::mutex> lock(les_mutex_);
        auto it = les_.find(k);
        if (it == les_.end()) {
            return nullptr;
        }
        return it->second;
    }
}
}
