#ifndef MATECFG_FRAGMENTS_CFG_H
#define MATECFG_FRAGMENTS_CFG_H


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

struct fragments_cfg_data {
	int id;
	std::vector<int> effect;
};
REFLECTION(fragments_cfg_data, id, effect)

struct fragments_cfg {
	std::unordered_map<int, fragments_cfg_data> data;
};
REFLECTION(fragments_cfg, data)

}
#endif
