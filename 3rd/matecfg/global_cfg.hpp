#ifndef MATECFG_GLOBAL_CFG_H
#define MATECFG_GLOBAL_CFG_H


#include "3rd/iguana/json.hpp"
#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace matecfg {

struct global_cfg_data {
	int id;
	int value;
	std::string desc;
};
REFLECTION(global_cfg_data, id, value, desc)

struct global_cfg {
	std::unordered_map<int, global_cfg_data> data;
};
REFLECTION(global_cfg, data)

}
#endif
