#ifndef TEST_CONFIGCENTER_H
#define TEST_CONFIGCENTER_H

#include "frmpub/configcenter.h"
#include "frmpub/matecfg.h"
#include "shynet/utils/singleton.h"
#include "shynet/utils/stringify_stl.h"
#include <optional>

void test_configcenter()
{
    auto& cfgmgr = shynet::utils::Singleton<frmpub::ConfigCenter>::instance();
    cfgmgr.set_search_path("lua/config");
    cfgmgr.load<frmpub::task_cfg>();
    const std::optional<frmpub::task_cfg_record> record = cfgmgr.get<frmpub::task_cfg, frmpub::task_cfg_record>(1010000);
    if (record) {
        std::cout << record->Id << " " << record->KeyName << " " << record->AcceptCond << std::endl;
        //std::cout << shynet::utils::stringify::stl(record->Ints) << std::endl;
    }
    //cfgmgr.reload<frmpub::task_cfg>();

    cfgmgr.for_each<frmpub::task_cfg>([](const int& k, const frmpub::task_cfg_record& v) {
        std::cout << k << std::endl;
    });
}
#endif
