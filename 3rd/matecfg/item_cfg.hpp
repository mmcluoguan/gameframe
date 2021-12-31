#ifndef MATECFG_ITEM_CFG_H
#define MATECFG_ITEM_CFG_H


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

struct item_cfg_data {
	int id;
	std::string name;
	int type;
	int overlap;
};
REFLECTION(item_cfg_data, id, name, type, overlap)

struct item_cfg {
	std::unordered_map<int, item_cfg_data> data;
};
REFLECTION(item_cfg, data)

}
#endif
