#ifndef DBVISIT_DATATIMERMGR_H
#define DBVISIT_DATATIMERMGR_H

#include "shynet/utils/Singleton.h"
#include <unordered_map>

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

#endif
