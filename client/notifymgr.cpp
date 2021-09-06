#include "notifymgr.h"
#include <unistd.h>
#include <cstring>
#include <sys/inotify.h>
#include <sys/select.h>
#include <dirent.h>

namespace utils
{

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 512 * ( EVENT_SIZE + 32 ) )

	NotifyMgr::NotifyMgr()
	{
		m_notifyfd = inotify_init();
		m_changefile = new std::set<std::string>();
	}

	static void run(NotifyMgr* mgr)
	{
		fd_set set;
		while (mgr->isstart())
		{
			FD_ZERO(&set);
			FD_SET(mgr->notifyfd(), &set);
			struct timeval val { 1, 0 };
			int nready = select(FD_SETSIZE, &set, NULL, NULL, &val);
			if (nready == -1)
			{
				perror("error select !");
				exit(-1);
			}
			else if (nready == 0) {
				continue;
			}
			if (FD_ISSET(mgr->notifyfd(), &set))
			{
				mgr->handle_read();
			}
		}
	}

	void NotifyMgr::start()
	{
		if (!m_isstart)
		{			
			m_isstart = true;
			m_thread = new std::thread(run, this);
		}
	}

	void NotifyMgr::stop()
	{
		if (m_isstart)
		{
			m_isstart = false;
			m_thread->join();
			delete m_thread;
			for (auto& it : m_fdpath) {
				inotify_rm_watch(m_notifyfd, it.first);
			}
			if(m_changefile)
			{
				delete m_changefile;
			}
		}
	}	

	void NotifyMgr::handle_read()
	{
		char buffer[BUF_LEN];
		memset(buffer, 0, BUF_LEN);
		size_t length = read(m_notifyfd, buffer, (size_t)BUF_LEN);

		if (length < 0) {
			perror("read");
			exit(1);
		}

		size_t i = 0;
		while (i < length) {
			struct inotify_event* event = (struct inotify_event*)&buffer[i];
			if (event->len > 0) {
				std::lock_guard<std::mutex> lock(m_mutex);
				auto iter = m_fdpath.find(event->wd);
				if (iter != m_fdpath.end())
				{
					std::string name(event->name, event->len);
					m_changefile->insert(iter->second + "/" + name);
				}
			}
			i += EVENT_SIZE + event->len;
		}
		memset(buffer, 0, BUF_LEN);
	}

	typedef std::function<void(const char* fullpath, uint32_t mask)> callback;

	static void traversedir(const char* basePath, callback cb, bool ischild, uint32_t mask)
	{
		DIR* dir;
		struct dirent* ptr;

		if ((dir = opendir(basePath)) == NULL)
		{
			perror("open dir error...");
			exit(1);
		}

		while ((ptr = readdir(dir)) != NULL)
		{		
			if (strcmp(ptr->d_name, ".") == 0 || 
				strcmp(ptr->d_name, "..") == 0 )
				continue;
			else if (ptr->d_type == DT_REG || ptr->d_type == DT_DIR)
			{
				char base[NAME_MAX];
				memset(base, '\0', sizeof(base));
				strcpy(base, basePath);
				strcat(base, "/");
				strcat(base, ptr->d_name);
				cb(base, mask);
				if (ptr->d_type == DT_DIR && ischild) {
					traversedir(base, cb, ischild, mask);
				}
			}			
		}
		closedir(dir);
	}

	bool NotifyMgr::add(const char* path, bool ischild, uint32_t mask)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		int fd = inotify_add_watch(m_notifyfd, path, mask);
		m_fdpath[fd] = path;
		callback cb = std::bind(&NotifyMgr::onAdd, this, std::placeholders::_1, std::placeholders::_2);
		traversedir(path, cb, ischild, mask);		
		return true;
	}

	bool utils::NotifyMgr::remove(const char* path)
	{
		for (auto& it : m_fdpath)
		{
			if (strcmp(it.second.c_str(), path) == 0) {
				m_fdpath.erase(it.first);
				inotify_rm_watch(m_notifyfd, it.first);
				return true;
			}
		}
		return false;
	}

	void NotifyMgr::onAdd(const char* fullpath, uint32_t mask) {
		int fd = inotify_add_watch(m_notifyfd, fullpath, mask);
		m_fdpath[fd] = fullpath;
	}

	std::set<std::string>* NotifyMgr::get(std::set<std::string>* src) {
		std::set<std::string>* target = nullptr;
		if (src == nullptr)	{
			perror("src is null");
			exit(1);
		}
		else {
			src->clear();
			target = m_changefile;
			m_changefile = src;
		}
		return target;
	}
}