#pragma once
#include "shynet/Basic.h"
#include <list>
#include <map>

namespace shynet {
	namespace utils {
		/*固定大小的LRU cache，支持插入，查询，以及获取全量列表
			*
			```
			shynet::utils::Lru<std::string, int> c(3);
			c.put("chef", 1);
			c.put("yoko", 2);
			c.put("tom", 3);
			c.put("jerry", 4); // 超过容器大小，淘汰最老的`chef`
			bool exist;
			int v;
			exist = c.get("chef", &v);
			//assert(!exist);
			exist = c.get("yoko", &v);
			//assert(exist && v == 2);
			c.put("garfield", 5); // 超过容器大小，注意，由于`yoko`刚才读取时会更新热度，所以淘汰的是`tom`
			exist = c.get("yoko", &v);
			//assert(exist && v == 2);
			exist = c.get("tom", &v);
			//assert(!exist);
		*/

		template <typename KeyT, typename ValueT>
		class Lru : public Nocopy {
		public:
			using KvPair = std::pair<KeyT, ValueT>;
			using List = std::list<KvPair>;

		public:
			// @param cap 容器大小
			explicit Lru(std::size_t cap) : capacity_(cap) {

			}
			~Lru() {
				list_.clear();
				map_.clear();
			}

		public:
			// @NOTICE function put 和 function get 操作都会更新元素热度，put 的 key 即使已经存在甚至对应的 value 相同也会更新热度

			// 插入前k不存在返回true，否则返回false
			bool put(KeyT k, ValueT v) {
				bool not_exist = true;
				typename Map::iterator iter = map_.find(k);
				if (iter != map_.end()) {
					list_.erase(iter->second);
					map_.erase(iter);
					not_exist = false;
				}

				list_.push_front(std::make_pair(k, v));
				map_[k] = list_.begin();

				if (list_.size() > capacity_) {
					KeyT old = list_.back().first;
					list_.pop_back();
					map_.erase(old);
				}
				return not_exist;
			}

			// k存在返回true，否则false
			bool get(KeyT k, ValueT* v) {
				typename Map::iterator iter = map_.find(k);
				if (iter == map_.end()) {
					return false;
				}
				KvPair kvp = *(iter->second);
				list_.erase(iter->second);
				list_.push_front(kvp);
				map_[k] = list_.begin();
				*v = kvp.second;
				return true;
			}

			// 获取整个列表
			List get_list() {
				return list_;
			}

			std::size_t size() const {
				return list_.size();
			}
			std::size_t capacity() const {
				return capacity_;
			}
		private:
			using Map = std::map<KeyT, typename List::iterator>;
			const std::size_t capacity_;
			List              list_;
			Map               map_;

		};
	}
}
