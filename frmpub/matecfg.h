#ifndef FRMPUB_MATECFG_H
#define FRMPUB_MATECFG_H

#include "3rd/iguana/json.hpp"
#include <array>
#include <deque>
#include <forward_list>
#include <fstream>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace frmpub {

/*===================================demo_cfg.josn映射===========================*/
struct demo_cfg_record_tag_record {
    int a;
    std::string b;
};

REFLECTION(demo_cfg_record_tag_record, a, b)

struct demo_cfg_record_reward {
    int diamom;
    int gold;
};

REFLECTION(demo_cfg_record_reward, diamom, gold)

struct demo_cfg_record {
    int id;
    std::string name;
    int type;
    std::vector<int32_t> accept_cond;
    std::string comp_cond;
    demo_cfg_record_reward reward;
    double power;
    std::list<demo_cfg_record_tag_record> tag;
};
REFLECTION(demo_cfg_record, id, name, type, accept_cond, comp_cond, reward, power, tag)

struct demo_cfg {
    std::unordered_map<int, demo_cfg_record> data;
};
REFLECTION(demo_cfg, data)

}

#endif
