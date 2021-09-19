#include "frmpub/LuaFolderTask.h"
#include "frmpub/LuaHotFixTask.h"
#include "shynet/lua/LuaEngine.h"
#include "shynet/pool/ThreadPool.h"
#include <chrono>

namespace frmpub {
	LuaFolderTask::LuaFolderTask(std::string path, bool ischild, uint32_t mask)
		: shynet::task::NotifyTask(path, ischild, mask) {
	}

	LuaFolderTask::~LuaFolderTask() {
	}

	/*
	* 临时记录文件最新修改时间，精确到毫秒
	* 用于判断文件修改间隔
	*/
	static std::unordered_map<std::string, int64_t> g_filemodifys;

	int LuaFolderTask::notify_event(std::string path, uint32_t mask) {
		if (mask & IN_MODIFY) {
			auto time_now = std::chrono::system_clock::now();
			auto duration_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_now.time_since_epoch());
			auto iter = g_filemodifys.find(path);
			bool ismodify = true;
			if (iter != g_filemodifys.end()) {
				//不是第一次修改文件,判断与上次修改间隔时间，间隔小于5毫秒不通知
				if (duration_in_ms.count() - iter->second < 5) {
					ismodify = false;
				}
			}
			if (ismodify) {
				LOG_DEBUG << path << " 被修改";
				shynet::utils::Singleton<shynet::lua::LuaEngine>::get_instance().append(
					std::make_shared<LuaHotFixTask>(path)
				);
				g_filemodifys[path] = duration_in_ms.count();
			}
		}
		else if (mask & IN_CREATE) {
			shynet::utils::Singleton<pool::ThreadPool>::get_instance().notifyTh().lock()->add(
				std::make_shared<LuaFolderTask>(path, true)
			);
			LOG_DEBUG << path << " 被加入监控";
		}
		return 0;
	}
}
