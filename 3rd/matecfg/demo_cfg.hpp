#ifndef MATECFG_DEMO_CFG_H
#define MATECFG_DEMO_CFG_H


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

struct demo_cfg_data_reward {
	int diamom;
	int gold;
};
REFLECTION(demo_cfg_data_reward, diamom, gold)

struct demo_cfg_data_tag {
	int a;
	std::string b;
};
REFLECTION(demo_cfg_data_tag, a, b)

struct demo_cfg_data {
	int id;
	std::string name;
	int type;
	std::vector<int> accept_cond;
	std::string comp_cond;
	demo_cfg_data_reward reward;
	double power;
	std::vector<demo_cfg_data_tag> tag;
};
REFLECTION(demo_cfg_data, id, name, type, accept_cond, comp_cond, reward, power, tag)

struct demo_cfg {
	std::unordered_map<int, demo_cfg_data> data;
};
REFLECTION(demo_cfg, data)

}
#endif
