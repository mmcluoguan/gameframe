#include "shynet/task/notifytask.h"
#include "shynet/net/timerreactormgr.h"

namespace shynet {
namespace task {

    NotifyTask::NotifyTask(std::string path, bool ischild, uint32_t mask)
    {
        path_ = path;
        ischild_ = ischild;
        mask_ = mask;
    }
    NotifyTask::~NotifyTask()
    {
    }
}
}
