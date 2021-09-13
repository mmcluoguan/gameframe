#pragma once
#include <mutex>
#include <unordered_map>
#include "shynet/utils/Singleton.h"

namespace dbvisit {
	class DataTimerMgr : public shynet::Nocopy {
		friend class shynet::utils::Singleton<DataTimerMgr>;
		DataTimerMgr();
	public:
		~DataTimerMgr();

		/*
		* 添加,删除,查找db保存计时器
		*/
		void add(std::string cachekey, int timerid);
		bool remove(std::string cachekey);
		int find(std::string cachekey);

	private:
		std::mutex cachekey_timerid_mutex_;
		std::unordered_map<std::string, int> cachekey_timerids_;
	};
}
