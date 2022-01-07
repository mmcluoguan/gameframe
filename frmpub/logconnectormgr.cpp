#include "frmpub/logconnectormgr.h"
#include "shynet/net/connectreactormgr.h"

namespace frmpub {
std::shared_ptr<LogConnector> LogConnectorMgr::log_connector()
{
    if (logctor_ids_.begin() != logctor_ids_.end()) {
        return find_logctor(*logctor_ids_.begin());
    }
    return nullptr;
}

void LogConnectorMgr::add_logctor(int connectid)
{
    std::lock_guard<std::mutex> lock(logctor_ids_mtx_);
    logctor_ids_.push_back(connectid);
}

void LogConnectorMgr::remove_logctor(int connectid)
{
    std::lock_guard<std::mutex> lock(logctor_ids_mtx_);
    logctor_ids_.remove(connectid);
}

std::shared_ptr<LogConnector> LogConnectorMgr::find_logctor(int connectid)
{
    return std::dynamic_pointer_cast<LogConnector>(
        shynet::utils::Singleton<net::ConnectReactorMgr>::instance().find(connectid));
}
}
