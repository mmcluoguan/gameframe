#include "shynet/net/connectreactormgr.h"
#include "shynet/pool/threadpool.h"
#include "shynet/thread/connectthread.h"
#include "shynet/utils/logger.h"

namespace shynet {
namespace net {

    int ConnectReactorMgr::add(std::shared_ptr<ConnectEvent> v)
    {
        static int connectid = 0;
        {
            std::lock_guard<std::mutex> lock(cnt_mutex_);
            connectid++;
            cnts_.insert({ connectid, v });
            v->set_connectid(connectid);
        }
        notify(connectid);
        return connectid;
    }

    bool ConnectReactorMgr::remove(int k)
    {
        std::lock_guard<std::mutex> lock(cnt_mutex_);
        return cnts_.erase(k) > 0 ? true : false;
    }

    std::shared_ptr<ConnectEvent> ConnectReactorMgr::find(int k)
    {
        std::lock_guard<std::mutex> lock(cnt_mutex_);
        auto it = cnts_.find(k);
        if (it == cnts_.end()) {
            return nullptr;
        }
        return it->second;
    }

    std::shared_ptr<ConnectEvent> ConnectReactorMgr::find(const std::string ip, unsigned short port)
    {
        std::lock_guard<std::mutex> lock(cnt_mutex_);
        for (auto&& [key, value] : cnts_) {
            if (value != nullptr) {
                if (value->connect_addr()->ip() == ip && value->connect_addr()->port() == port) {
                    return value;
                }
            }
        }
        return nullptr;
    }

    void ConnectReactorMgr::notify(int connectid)
    {
        std::shared_ptr<thread::ConnectThread> cnt = utils::Singleton<pool::ThreadPool>::instance().connectTh().lock();
        if (cnt != nullptr) {
            cnt->notify(connectid);
        } else
            LOG_WARN << "没有可用的 ConnectThread";
    }

}
}
