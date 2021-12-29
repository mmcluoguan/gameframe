#ifndef __SRC_UTILS_NOTIFYMGR_H__
#define __SRC_UTILS_NOTIFYMGR_H__

#include <sys/inotify.h>
#include <unordered_map>
#include <thread>
#include <set>
#include <mutex>

namespace utils
{
	class NotifyMgr final {
	public:
		NotifyMgr();
		NotifyMgr(const NotifyMgr&) = delete;
		NotifyMgr(NotifyMgr&) = delete;
		NotifyMgr(NotifyMgr&&) = delete;
		NotifyMgr& operator=(const NotifyMgr&) = delete;
		NotifyMgr& operator=(NotifyMgr&) = delete;
		NotifyMgr& operator=(NotifyMgr&&) = delete;

		void start();
		void stop();
		/*
		* 添加监控
		* path 文件或目录
		* ischild 如果是目录，是否监控子目录
		* mask 监控选项,默认创建和修改
		* 返回是否成功
		*/
		bool add(const char* path, bool ischild = false, uint32_t mask = IN_MODIFY | IN_CREATE);
		/*
		* 移除监控
		* path 文件或目录
		*/
		bool remove(const char* path);

		/*
		* 获取改变的文件 (返回的指针需要外部释放)
		* src 提供下次收集的容器
		* 返回值为改变的文件
		*/
		std::set<std::string>* get(std::set<std::string>* src);

		void onAdd(const char* fullpath, uint32_t mask);

		bool isstart() const {
			return m_isstart;
		}

		int notifyfd() const {
			return m_notifyfd;
		}

		void handle_read();
	private:
		std::thread* m_thread = nullptr;
		bool m_isstart = false;
		int m_notifyfd = 0;
		/*
		* fd与文件路径关联
		*/
		std::unordered_map<int, std::string> m_fdpath;
		/*
		* 改变的文件
		*/
		std::set<std::string>* m_changefile;
		std::mutex m_mutex;
	};
}
#endif