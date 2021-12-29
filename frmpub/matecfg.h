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

/*===================================task_cfg.josn”≥…‰===========================*/
struct task_cfg_record {
    int Id;
    std::string KeyName;
    std::vector<int> Ints;
};
REFLECTION(task_cfg_record, Id, KeyName, Ints)

struct task_cfg {
    std::unordered_map<int, task_cfg_record> data;
};
REFLECTION(task_cfg, data)

}

#endif
