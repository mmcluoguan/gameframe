#ifndef FRMPUB_LUAFOLDERTASK_H
#define FRMPUB_LUAFOLDERTASK_H

#include "shynet/task/NotifyTask.h"

namespace frmpub {
	/*
	* lua文件变化通知
	*/
	class LuaFolderTask : public shynet::task::NotifyTask {
	public:
		LuaFolderTask(std::string path, bool ischild = true, uint32_t mask = IN_MODIFY | IN_CREATE);
		~LuaFolderTask();

		/*
			* 监控目录发生事件
			*/
		int notify_event(std::string path, uint32_t mask) override;
	private:

	};
}

#endif
