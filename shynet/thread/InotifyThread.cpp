#include "shynet/thread/inotifythread.h"
#include "shynet/pool/threadpool.h"
#include "shynet/utils/logger.h"
#include "shynet/utils/stuff.h"
#include <fcntl.h>
#include <ftw.h>
#include <list>
#include <sys/inotify.h>

namespace shynet {
extern pthread_barrier_t g_barrier;

namespace thread {
    InotifyThread::InotifyThread(size_t index)
        : Thread(ThreadType::INOTIFY, index)
    {
        notifyfd_ = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
        if (notifyfd_ == -1) {
            THROW_EXCEPTION("call inotify_init");
        }
    }

    InotifyThread::~InotifyThread()
    {
        for (auto&& [key, value] : task_map_) {
            inotify_rm_watch(notifyfd_, key);
        }
    }

    static void ioreadcb(struct bufferevent* bev, void* ptr)
    {
        InotifyThread* p = reinterpret_cast<InotifyThread*>(ptr);
        try {
            p->io_readcb();
        } catch (const std::exception& err) {
            utils::Stuff::print_exception(err);
        }
    }

    void InotifyThread::io_readcb()
    {
        struct inotify_event ev;
        while (iobuf_->inputbuffer()->length() >= sizeof(struct inotify_event)) {
            iobuf_->inputbuffer()->remove(&ev, sizeof(struct inotify_event));
            if (ev.len > 0 && iobuf_->inputbuffer()->length() >= ev.len) {
                char filename[NAME_MAX + 1] = { 0 };
                iobuf_->inputbuffer()->remove(filename, ev.len);
                std::unordered_map<int, std::shared_ptr<task::NotifyTask>>::iterator iter;
                {
                    //找到wd关联的task
                    std::lock_guard<std::mutex> lock(task_mutex_);
                    iter = task_map_.find(ev.wd);
                }
                if (iter != task_map_.end()) {
                    auto& fdpaths = iter->second->fdptahs();
                    //找到wd关联的目录路径
                    std::unordered_map<int, std::string>::iterator fdpaths_iter;
                    {
                        std::lock_guard<std::mutex> lock(task_mutex_);
                        fdpaths_iter = fdpaths.find(ev.wd);
                    }
                    if (fdpaths_iter != fdpaths.end()) {
                        std::string path = fdpaths_iter->second + "/" + filename;
                        if (ev.mask & IN_ACCESS)
                            LOG_TRACE << "文件被访问 " << path;
                        if (ev.mask & IN_ATTRIB)
                            LOG_TRACE << "文件属性发生变化 " << path;
                        if (ev.mask & IN_CLOSE_NOWRITE)
                            LOG_TRACE << "以非可写的方式打开后关闭了文件 " << path;
                        if (ev.mask & IN_CLOSE_WRITE)
                            LOG_TRACE << "以可写的方式打开后关闭了文件 " << path;
                        if (ev.mask & IN_CREATE)
                            LOG_TRACE << "监控目录下新建了子文件、子目录 " << path;
                        if (ev.mask & IN_DELETE)
                            LOG_TRACE << "监控目录下删除了子文件、子目录 " << path;
                        if (ev.mask & IN_DELETE_SELF)
                            LOG_TRACE << "监控目录被删除 " << path;
                        if (ev.mask & IN_IGNORED)
                            LOG_TRACE << "文件被忽略  " << path;
                        if (ev.mask & IN_ISDIR)
                            LOG_TRACE << "针对目录发生的事件 " << path;
                        if (ev.mask & IN_MODIFY)
                            LOG_TRACE << "文件被修改 " << path;
                        if (ev.mask & IN_MOVED_FROM)
                            LOG_TRACE << "文件移出监控目录 " << path;
                        if (ev.mask & IN_MOVED_TO)
                            LOG_TRACE << "文件移入监控目录 " << path;
                        if (ev.mask & IN_OPEN)
                            LOG_TRACE << "文件被打开 " << path;
                        if (ev.mask & IN_Q_OVERFLOW)
                            LOG_TRACE << "事件队列溢出 " << path;
                        if (ev.mask & IN_MOVE_SELF)
                            LOG_TRACE << "监控目录被移动 " << path;
                        if (ev.mask & IN_UNMOUNT)
                            LOG_TRACE << "支持文件系统已卸载 " << path;

                        iter->second->notify_event(path, ev.mask);
                    }
                }
            }
        }
    }

    //借用全局变量作为子目录遍历临时变量
    static thread_local int g_notifyfd = 0;
    static thread_local task::NotifyTask* g_task = nullptr;
    static thread_local std::list<int> g_wds;
    static thread_local bool g_ischild = false;

    void InotifyThread::add(std::shared_ptr<task::NotifyTask> task)
    {

        g_notifyfd = notifyfd_;
        g_ischild = task->ischild();
        g_task = task.get();
        std::lock_guard<std::mutex> lock(task_mutex_);
        if (nftw(
                task->path().c_str(), [](const char* pathname, const struct stat* sbuf, int type, struct FTW* ftwb) -> int {
                    if (type == FTW_D) {
                        if (ftwb->level != 0 && !g_ischild) {
                            return FTW_SKIP_SUBTREE;
                        }
                    }
                    if (type == FTW_D || type == FTW_F) {
                        int fd = inotify_add_watch(g_notifyfd, pathname, g_task->mask());
                        g_wds.push_back(fd);
                        g_task->fdptahs().insert({ fd, pathname });
                        LOG_TRACE << "inotify path:" << pathname << " fd:" << fd;
                    }
                    return FTW_CONTINUE;
                },
                10, FTW_MOUNT | FTW_PHYS)
            == -1) {
            THROW_EXCEPTION("call nftw");
        }
        for (int fd : g_wds) {
            task_map_[fd] = task;
        }
        g_wds.clear();
        g_notifyfd = 0;
        g_ischild = false;
        g_task = nullptr;
    }

    void InotifyThread::remove(std::shared_ptr<task::NotifyTask> task)
    {
        for (auto&& [key, value] : task->fdptahs()) {
            inotify_rm_watch(notifyfd_, key);
            std::lock_guard<std::mutex> lock(task_mutex_);
            task_map_.erase(key);
        }
        task->fdptahs().clear();
    }

    int InotifyThread::run()
    {
        try {
            base_ = std::shared_ptr<events::EventBase>(new events::EventBase());
            iobuf_ = std::shared_ptr<events::EventBuffer>(new events::EventBuffer(base_, notifyfd_, 0));

            iobuf_->setcb(ioreadcb, nullptr, nullptr, this);
            iobuf_->enabled(EV_READ | EV_PERSIST);
            pthread_barrier_wait(&g_barrier);
            base_->loop(EVLOOP_NO_EXIT_ON_EMPTY);
        } catch (const std::exception& err) {
            utils::Stuff::print_exception(err);
        }
        return 0;
    }

    int InotifyThread::stop()
    {
        base_->loopexit();
        return 0;
    }

}
}
