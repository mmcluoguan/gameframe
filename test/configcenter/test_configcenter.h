#ifndef TEST_CONFIGCENTER_H
#define TEST_CONFIGCENTER_H

#include "3rd/matecfg/demo_cfg.hpp"
#include "frmpub/configcenter.h"
#include "shynet/utils/singleton.h"
#include <optional>

namespace shynet::utils::stringify {
std::string stl_one_(const matecfg::demo_cfg_data_tag& record)
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
    cfgmgr.load<matecfg::demo_cfg>();
    const std::optional<matecfg::demo_cfg_data> record = cfgmgr.get<matecfg::demo_cfg, matecfg::demo_cfg_data>(1010000);
    //cfgmgr.reload<matecfg::demo_cfg>();

    cfgmgr.for_each<matecfg::demo_cfg>([](const int& k, const matecfg::demo_cfg_data& v) {
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
