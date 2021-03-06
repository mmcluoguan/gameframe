#include "shynet/pool/threadpool.h"
#include "shynet/utils/logger.h"
#include "shynet/utils/stuff.h"

namespace shynet {

pthread_barrier_t g_barrier;

namespace pool {

    ThreadPool::ThreadPool(size_t workNum, size_t acceptNum)
    {
        acceptNum_ = acceptNum;
        workNum_ = workNum;
    }

    ThreadPool::~ThreadPool()
    {
        stop();
    }

    void ThreadPool::start()
    {
        static std::once_flag oc;
        std::call_once(oc,
            [&]() {
                pthread_barrier_init(&g_barrier, nullptr, 6);
                size_t i = 0;
                std::shared_ptr<thread::ListenThread> shlisten = std::make_shared<thread::ListenThread>(i);
                listernTh_ = shlisten;
                std::thread::id id = shlisten->start()->get_id();
                tifs_.push_back(shlisten);
                LOG_TRACE << "ListenThread start [" << i << "]:" << std::hash<std::thread::id>()(id);

                for (size_t n = 0; n < acceptNum_; n++) {
                    i++;
                    std::shared_ptr<thread::AcceptThread> acceptTh = std::make_shared<thread::AcceptThread>(i);
                    id = acceptTh->start()->get_id();
                    tifs_.push_back(acceptTh);
                    LOG_TRACE << "AcceptThread start [" << i << "]:" << std::hash<std::thread::id>()(id);
                    acceptThs_.push_back(acceptTh);
                }

                for (size_t n = 0; n < workNum_; n++) {
                    i++;
                    std::shared_ptr<thread::WorkThread> workerTh = std::make_shared<thread::WorkThread>(i);
                    id = workerTh->start()->get_id();
                    tifs_.push_back(workerTh);
                    LOG_TRACE << "WorkThread start [" << i << "]:" << std::hash<std::thread::id>()(id);
                    workThs_.push_back(workerTh);
                }

                i++;
                std::shared_ptr<thread::TimerThread> shtimer = std::make_shared<thread::TimerThread>(i);
                timerTh_ = shtimer;
                id = shtimer->start()->get_id();
                tifs_.push_back(shtimer);
                LOG_TRACE << "TimerThread start [" << i << "]:" << std::hash<std::thread::id>()(id);

                i++;
                std::shared_ptr<thread::ConnectThread> shconnect = std::make_shared<thread::ConnectThread>(i);
                connectTh_ = shconnect;
                id = shconnect->start()->get_id();
                tifs_.push_back(shconnect);
                LOG_TRACE << "ConnectThread start [" << i << "]:" << std::hash<std::thread::id>()(id);

                i++;
                std::shared_ptr<thread::LuaThread> shlua = std::make_shared<thread::LuaThread>(i);
                luaTh_ = shlua;
                id = shlua->start()->get_id();
                tifs_.push_back(shlua);
                LOG_TRACE << "LuaThread start [" << i << "]:" << std::hash<std::thread::id>()(id);

                i++;
                std::shared_ptr<thread::InotifyThread> shnotify = std::make_shared<thread::InotifyThread>(i);
                notifyTh_ = shnotify;
                id = shnotify->start()->get_id();
                tifs_.push_back(shnotify);
                LOG_TRACE << "InotifyThread start [" << i << "]:" << std::hash<std::thread::id>()(id);

                i++;
                std::shared_ptr<thread::UdpThread> udp = std::make_shared<thread::UdpThread>(i);
                udpTh_ = udp;
                id = udp->start()->get_id();
                tifs_.push_back(udp);
                LOG_TRACE << "UdpThread start [" << i << "]:" << std::hash<std::thread::id>()(id);

                //??????ListenThread,ConnectThread,TimerThread,LuaThread,InotifyThread????????????
                pthread_barrier_wait(&g_barrier);
            });
    }

    void ThreadPool::stop()
    {
        for (const auto& it : tifs_) {
            it->stop();
        }
        {
            std::lock_guard<std::mutex> lock(tasks_mutex_);
            tasks_condvar_.notify_all();
        }
        int i = 0;
        for (const auto& it : tifs_) {
            std::thread::id id = it->thread()->get_id();
            LOG_TRACE << "over thread:[" << i << "]:" << std::hash<std::thread::id>()(id) << " wait";
            try {
                if (it->thread()->joinable())
                    it->thread()->join();
            } catch (const std::exception& err) {
                LOG_WARN << "over thread:[" << i << "]" << std::hash<std::thread::id>()(id) << " abort";
                //???????????????????????????
                it->thread()->detach();
            }
            LOG_TRACE << "over thread:[" << i << "]:" << std::hash<std::thread::id>()(id) << " complete";
            i++;
        }
        tifs_.clear();
    }
}
}
