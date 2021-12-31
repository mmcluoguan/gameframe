#ifndef TEST_CONFIGCENTER_H
#define TEST_CONFIGCENTER_H

#include "frmpub/configcenter.h"
#include "frmpub/matecfg.h"
#include "shynet/utils/singleton.h"
#include <optional>

namespace shynet::utils::stringify {
std::string stl_one_(const frmpub::demo_cfg_record_tag_record& record)
{
    std::ostringstream oss;
    oss << "(" << record.b << "," << record.a << ")";
    return oss.str();
}
}
#include "shynet/utils/stringify_stl.h"

void test_configcenter()
{
    auto& cfgmgr = shynet::utils::Singleton<frmpub::ConfigCenter>::instance();
    cfgmgr.set_search_path("lua/config");
    cfgmgr.load<frmpub::demo_cfg>();
    const std::optional<frmpub::demo_cfg_record> record = cfgmgr.get<frmpub::demo_cfg, frmpub::demo_cfg_record>(1010000);
    //cfgmgr.reload<frmpub::task_cfg>();

    cfgmgr.for_each<frmpub::demo_cfg>([](const int& k, const frmpub::demo_cfg_record& v) {
        std::cout << v.id << ","
                  << v.name << ","
                  << v.power << ","
                  << v.reward.diamom << ","
                  << v.reward.gold << ","
                  << std::endl;
        std::cout << shynet::utils::stringify::stl(v.accept_cond) << std::endl;
        std::cout << shynet::utils::stringify::stl(v.tag) << std::endl;
    });
}
#endif
