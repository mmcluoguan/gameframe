#ifndef MATECFG_TASK_CFG_H
#define MATECFG_TASK_CFG_H


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

struct task_cfg_data {
	int id;
	std::string name;
	int type;
	std::string accept_cond;
	std::string comp_cond;
	std::string reward;
};
REFLECTION(task_cfg_data, id, name, type, accept_cond, comp_cond, reward)

struct task_cfg {
	std::unordered_map<int, task_cfg_data> data;
};
REFLECTION(task_cfg, data)

}
#endif
