#ifndef SHYNET_UTILS_SKIPLIST_H
#define SHYNET_UTILS_SKIPLIST_H

#include <chrono>
#include <functional>
#include <iostream>
#include <random>
#include <vector>
#include <unordered_map>

namespace shynet {
	namespace utils {

		/*
		* 默认按照score降序排序，score相同按照key降序排序
		*/
		template <typename Key, typename Score>
		static int default_compare(const std::pair<const Key, Score>& a, const std::pair<const Key, Score>& b) {
			//>升序 <降序
			if (a.second < b.second) {
				return 1;
			}
			else if (a.second == b.second) {
				if (a.first >= b.first) {
					return 1;
				}
				return -1;
			}
			else {
				return -1;
			}
		}

		//
		// 跳表
		// key不能相同，不同的key可以有相同的分数
		//
		template <typename Key, typename Score, typename Compare = decltype(std::bind(default_compare<Key, Score>, std::placeholders::_1, std::placeholders::_2)) >
		class SkipList {

		public:
			using key_type = Key;
			using value_type = std::pair<const Key, Score>;

		private:
			/*
			* 内部节点
			*/
			struct node {
			public:
				friend class SkipList;

				struct level_item {
					node* prev_ = nullptr;
					node* next_ = nullptr;
					uint32_t span_ = 0;
				};
			public:
				explicit node(int height)
					: height_(height), levels_(height) {
				}

				explicit node(int height, const value_type& kv)
					: kv_(kv), height_(height), levels_(height) {
				}

			private:
				value_type kv_;
				int height_;
				std::vector<level_item> levels_;
			};
		public:
			/*
			* 正向迭代器
			*/
			class iterator {
				friend class SkipList;
			public:
				iterator() = default;
				explicit iterator(node* n) : node_(n) {}
				iterator(const iterator& other) { node_ = other.node_; }
				iterator& operator=(const iterator& other) {
					node_ = other.node_;
					return *this;
				}
				bool operator==(const iterator& other) const { return this->node_ == other.node_; }
				bool operator!=(const iterator& other) const { return this->node_ != other.node_; }
				value_type& operator*() const { return node_->kv_; }
				value_type* operator->() const { return &(node_->kv_); }

				iterator& operator++() {
					node_ = node_->levels_[0].next_;
					return *this;
				}

				iterator operator++(int) {
					node* keeper = node_;
					node_ = node_->levels_[0].next_;
					return iterator(keeper);
				}

				iterator& operator--() {
					node_ = node_->levels_[0].prev_;
					return *this;
				}

				iterator operator--(int) {
					node* keeper = node_;
					node_ = node_->levels_[0].prev_;
					return iterator(keeper);
				}
			private:
				node* node_ = nullptr;
			};
			/*
			* 逆向迭代器
			*/
			class reverse_iterator {
				friend class SkipList;
			public:
				reverse_iterator() = default;
				explicit reverse_iterator(node* n) : node_(n) {}
				reverse_iterator(const reverse_iterator& other) { node_ = other.node_; }
				reverse_iterator& operator=(const reverse_iterator& other) {
					node_ = other.node_;
					return *this;
				}
				bool operator==(const reverse_iterator& other) const { return this->node_ == other.node_; }
				bool operator!=(const reverse_iterator& other) const { return this->node_ != other.node_; }
				value_type& operator*() const { return node_->kv_; }
				value_type* operator->() const { return &(node_->kv_); }

				reverse_iterator& operator++() {
					node_ = node_->levels_[0].prev_;
					if (node_->levels_[0].prev_ == nullptr) { node_ = nullptr; }

					return *this;
				}

				reverse_iterator operator++(int) {
					node* keeper = node_;
					node_ = node_->levels_[0].prev_;
					if (node_->levels_[0].prev_ == nullptr) { node_ = nullptr; }

					return reverse_iterator(keeper);
				}

				reverse_iterator& operator--() {
					node_ = node_->levels_[0].next_;
					return *this;
				}

				reverse_iterator operator--(int) {
					node* keeper = node_;
					node_ = node_->levels_[0].next_;
					return reverse_iterator(keeper);
				}
			private:
				node* node_ = nullptr;
			};

			/// <summary>
			/// 空构造
			/// </summary>
			/// <param name="compare">排序函数</param>
			SkipList(Compare compare = std::bind(default_compare<Key, Score>, std::placeholders::_1, std::placeholders::_2))
				: random_generator_(std::chrono::system_clock::now().time_since_epoch().count()), compare_(compare) {
				head_ = new node(max_height_);
			}

			/// <summary>
			/// 区间构造
			/// </summary>
			/// <param name="first"></param>
			/// <param name="last"></param>
			/// <param name="compare"></param>
			SkipList(iterator first, iterator last, Compare compare = std::bind(default_compare<Key, Score>, std::placeholders::_1, std::placeholders::_2))
				: random_generator_(std::chrono::system_clock::now().time_since_epoch().count()), compare_(compare) {
				head_ = new node(max_height_);
				insert_range_effective_(first, last);
			}

			/*
			* 复制构造
			*/
			SkipList(const SkipList& x)
				: random_generator_(std::chrono::system_clock::now().time_since_epoch().count()), compare_(x.compare_) {
				head_ = new node(max_height_);
				insert_range_effective_(x.begin(), x.end());
			}

			/*
			* 移动构造
			*/
			SkipList(SkipList&& x)
				: random_generator_(std::chrono::system_clock::now().time_since_epoch().count()), compare_(x.compare_) {
				head_ = new node(max_height_);
				swap(x);
			}

			virtual ~SkipList() {
				clear();
				delete head_;
			}

			SkipList& operator=(const SkipList& x) {
				clear();
				insert_range_effective_(x.begin(), x.end());
				return *this;
			}

			SkipList& operator=(SkipList&& x) {
				clear();
				swap(x);
				return *this;
			}

			iterator begin() { return iterator(head_->levels_[0].next_); }
			iterator begin() const { return iterator(head_->levels_[0].next_); }
			iterator end() { return iterator(nullptr); }
			iterator end() const { return iterator(nullptr); }

			reverse_iterator rbegin() { return reverse_iterator(tail_); }
			reverse_iterator rbegin() const { return reverse_iterator(tail_); }
			reverse_iterator rend() { return reverse_iterator(nullptr); }
			reverse_iterator rend() const { return reverse_iterator(nullptr); }

			bool empty() const { return element_num_ == 0; }
			std::size_t size() const { return element_num_; }

			/*
			* 通过key获取score,没有找到key,抛出异常
			*/
			Score& at(const key_type& k) {
				iterator iter = find(k);
				if (iter == end()) {
					std::ostringstream err;
					err << "key=" << k << " 不存在";
					THROW_EXCEPTION(err.str());
				}

				return iter->second;
			}
			const Score& at(const key_type& k) const {
				iterator iter = find(k);
				if (iter == end()) {
					std::ostringstream err;
					err << "key=" << k << " 不存在";
					THROW_EXCEPTION(err.str());
				}

				return iter->second;
			}

			/*
			* 清空
			*/
			void clear() {
				for (int i = max_height_ - 1; i > 0; i--) {
					head_->levels_[i].next_ = nullptr;
				}

				node* p = head_->levels_[0].next_;
				while (p) {
					node* next = p->levels_[0].next_;
					delete p;
					p = next;
				}
				head_->levels_[0].next_ = nullptr;
				current_height_ = 1;
			}

			/*
			* 指定key,更新score,skiplist顺序将改变
			*/
			std::pair<iterator, bool> update(const key_type& k, const Score& score) {
				auto iter = find(k);
				if (iter != end()) {
					if (iter->second == score) {
						return std::make_pair(end(), false);
					}
					else {
						erase(iter);
						value_type val(k, score);
						return insert(val);
					}
				}
				return std::make_pair(end(), false);
			}
			/*
			* 指定position,更新score,skiplist顺序将改变
			*/
			std::pair<iterator, bool> update(iterator position, const Score& score) {
				if (position->second == score) {
					return std::make_pair(end(), false);
				}
				else {
					key_type key = position->first;
					erase(position);
					return insert({ key ,score });
				}
			}

			/*
			* 插入val
			*/
			std::pair<iterator, bool> insert(const value_type& val) {
				std::vector<node*> cache;
				iterator iter = find_(val, nullptr, nullptr, &cache);
				if (iter != end()) { return std::make_pair(iter, false); }

				iter = insert_(val, cache);
				return std::make_pair(iter, true);
			}
			/*
			* 在指定位置插入val
			*/
			iterator insert(iterator position, const value_type& val) {
				std::vector<node*> cache;
				iterator iter = find_(val, position.node_, nullptr, &cache);
				if (iter != end()) { return iter; }

				iter = find_(val, nullptr, position.node_, &cache);
				if (iter != end()) { return iter; }

				return insert_(val, cache);
			}
			/*
			* 插入区间值
			*/
			void insert(iterator first, iterator last) {
				insert_range_effective_(first, last);
			}

			/*
			* 交换skiplist
			*/
			void swap(SkipList& x) {
				std::swap(head_, x.head_);
				std::swap(tail_, x.tail_);
				std::swap(current_height_, x.current_height_);
				std::swap(element_num_, x.element_num_);
				std::swap(is_asc_, x.is_asc_);
				std::swap(kvmap_, x.kvmap_);
			}

			/*
			* 通过指定排名获取位置
			*/
			iterator rank_pos(uint32_t rank) const {
				if (rank == 1)
					return begin();
				else if (rank > element_num_)
					return end();
				node* dummy = head_;
				int h = current_height_;

				uint32_t target = 0;
				for (int i = h - 1; i >= 0; i--) {
					while (dummy->levels_[i].next_) {
						target += dummy->levels_[i].next_->levels_[i].span_;
						if (target == rank) {
							return iterator(dummy->levels_[i].next_);
						}
						else if (target > rank) {
							target -= dummy->levels_[i].next_->levels_[i].span_;
							dummy = dummy->levels_[i].next_->levels_[i].prev_;
							break;
						}
						dummy = dummy->levels_[i].next_;
					}
				}
				return end();
			}
			/*
			* 通过指定位置获取排名
			*/
			uint32_t pos_rank(const iterator it) const {
				node* next = head_->levels_[it.node_->height_ - 1].next_;
				uint32_t rank = 0;
				while (next) {
					rank += next->levels_[it.node_->height_ - 1].span_;
					next = next->levels_[it.node_->height_ - 1].next_;
				}
				return rank;
			}
			/*
			* 通过指定起始排名到结束排名的前一名，获取排名区间[beg_rank,end_rank)
			*/
			std::pair<iterator, iterator> rank_rang(uint32_t beg_rank, uint32_t end_rank) const {
				std::pair<iterator, iterator> rang = std::make_pair(end(), end());
				if (end_rank > beg_rank) {
					rang.first = rank_pos(beg_rank);
					rang.second = rank_pos(end_rank);
				}
				return rang;
			}

			/*
			* 通过k获取位置
			*/
			iterator find(const key_type& k) const {
				auto kv_iter = kvmap_.find(k);
				if (kv_iter != kvmap_.end()) {
					value_type val(kv_iter->first, kv_iter->second);
					return find_(val);
				}
				return end();
			}
			/*
			* 通过指定[min,max]获取位置区间
			*/
			std::pair<iterator, iterator> find(const Score& min, const Score& max, size_t max_item_size = 100) {
				std::pair<iterator, iterator> rang = std::make_pair(end(), end());
				if (max >= min) {
					node* dummy = head_;
					for (int i = current_height_ - 1; i >= 0; i--) {
						while (dummy->levels_[i].next_ &&
							((is_asc_ == -1 && dummy->levels_[i].next_->kv_.second > max) ||
								(is_asc_ == 1 && dummy->levels_[i].next_->kv_.second < min))
							) {
							dummy = dummy->levels_[i].next_;
						}
					}
					size_t total = 0;
					dummy = dummy->levels_[0].next_;
					while (dummy &&
						((is_asc_ == -1 && dummy->kv_.second >= min) ||
							(is_asc_ == 1 && dummy->kv_.second <= max))
						) {
						if (total == 0) {
							rang.first = iterator(dummy);
						}
						dummy = dummy->levels_[0].next_;
						total++;
						if (total >= max_item_size) {
							break;
						}
					}
					if (rang.first != end())
						rang.second = iterator(dummy);
				}
				return rang;
			}

			/*
			* 通过位置移除元素,返回移除元素的下一个位置
			*/
			iterator erase(iterator position) { return erase_(position); }
			/*
			* 通过key移除元素,返回移除元素的下一个位置
			*/
			iterator erase(const key_type& k) {
				auto kv_iter = kvmap_.find(k);
				if (kv_iter != kvmap_.end()) {
					value_type val(kv_iter->first, kv_iter->second);
					iterator iter = find_(val);
					if (iter == end()) {
						return end();
					}
					return erase_(iter);
				}
				return end();
			}
			/*
			* 通过指定区间移除元素
			*/
			void erase(iterator first, iterator last) {
				while (first != last) {
					first = erase(first);
				}
			}

			/*
			* 调试
			*/
			const std::string debug_string() {
				std::ostringstream info;
				info << "<<<<<" << std::endl;
				for (int i = current_height_ - 1; i >= 0; i--) {
					info << " * ";
					node* p = head_;
					for (; p->levels_[i].next_; p = p->levels_[i].next_) {
						info << "(" << p->levels_[i].next_->kv_.first << ":" << p->levels_[i].next_->kv_.second
							<< " span:" << p->levels_[i].next_->levels_[i].span_ << ") ";
					}
					info << std::endl;
				}
				info << "-----" << std::endl;
				return info.str();
			}
		private:
			iterator erase_(iterator it) {
				node* n = it.node_;

				iterator erase_next = end();
				for (int i = 0; i <= current_height_ - 1; i++) {
					if (i < n->height_) {
						erase_next = iterator(n->levels_[i].next_);
						n->levels_[i].prev_->levels_[i].next_ = n->levels_[i].next_;
						if (n->levels_[i].next_) {
							n->levels_[i].next_->levels_[i].prev_ = n->levels_[i].prev_;
						}
					}
					else {
						node* next = head_->levels_[i].next_;
						while (next) {
							int res = compare_(n->kv_, next->kv_);
							if (res < 0) { break; }
							else if (res > 0) { next = next->levels_[i].next_; }
						}
						if (next != nullptr)
							next->levels_[i].span_--;
					}
				}

				if (n->height_ != 1 && n->height_ == current_height_) {
					for (; (current_height_ > 1) && (head_->levels_[current_height_ - 1].next_ == nullptr); current_height_--);
				}

				if (n == tail_) { tail_ = n->levels_[0].prev_ == head_ ? nullptr : n->levels_[0].prev_; }

				element_num_--;
				kvmap_.erase(n->kv_.first);
				delete n;
				return erase_next;
			}

			iterator find_(const value_type& k, node* bpos = nullptr, node* epos = nullptr, std::vector<node*>* cache = nullptr) const {
				if (cache) { cache->reserve(max_height_); }

				node* dummy = bpos ? bpos->levels_[0].prev_ : head_;
				int h = bpos ? dummy->height_ : current_height_;

				for (int i = h - 1; i >= 0; i--) {
					for (; ; ) {
						if (epos == nullptr && dummy->levels_[i].next_ == nullptr) { break; }
						else if (epos != nullptr && (dummy->levels_[i].next_ == epos || dummy->levels_[i].next_ == nullptr)) { break; }
						if (k.first == dummy->levels_[i].next_->kv_.first) {
							return iterator(dummy->levels_[i].next_);
						}
						else {
							int res = compare_(k, dummy->levels_[i].next_->kv_);
							if (res == 0) {
								std::ostringstream err;
								err << "res不能为0";
								THROW_EXCEPTION(err.str());
							}
							else if (res < 0) { break; }
							else if (res > 0) { dummy = dummy->levels_[i].next_; }
						}
					}
					if (cache) { (*cache)[i] = dummy; }
				}

				return end();
			}

			iterator insert_(const value_type& kv, std::vector<node*>& cache) {
				kvmap_[kv.first] = kv.second;
				int height = random_height_();

				if (height > current_height_) {
					for (int i = height - 1; i >= current_height_; i--) {
						cache[i] = head_;
					}
					current_height_ = height;
				}

				//比较输入值与位置关系，确定跳表顺序
				if (is_asc_ == 0 && cache[0] != head_) {
					if (kv.second > cache[0]->kv_.second) {
						is_asc_ = 1;
					}
					else {
						is_asc_ = -1;
					}
				}

				node* nn = new node(height, kv);
				node* newptr = nn;
				for (int i = 0; i <= current_height_ - 1; i++) {
					node* cn = cache[i];
					if (i < height) {
						nn->levels_[i].next_ = cn->levels_[i].next_;
						nn->levels_[i].prev_ = cn;
						nn->levels_[i].span_ = 1;

						if (cn->levels_[i].next_) {
							cn->levels_[i].next_->levels_[i].prev_ = nn;
						}
						cn->levels_[i].next_ = nn;
					}
					else {
						nn = cn;
					}

					if (i != 0) {
						node* prev = nn->levels_[0].prev_;
						if (prev != nullptr
							&& nn->levels_[i].prev_ != nullptr) {
							uint32_t span = 1;
							while (prev != nullptr
								&& prev != nn->levels_[i].prev_) {
								span++;
								prev = prev->levels_[0].prev_;
							}
							nn->levels_[i].span_ = span;
						}

						node* next = nn->levels_[0].next_;
						if (next != nullptr
							&& nn->levels_[i].next_ != nullptr) {
							uint32_t span = 1;
							while (next != nullptr
								&& next != nn->levels_[i].next_) {
								span++;
								next = next->levels_[0].next_;
							}
							nn->levels_[i].next_->levels_[i].span_ = span;
						}
					}
				}

				if (newptr->levels_[0].next_ == nullptr) { tail_ = newptr; }

				element_num_++;
				return iterator(newptr);
			}

			void insert_range_effective_(iterator b, iterator e) {
				iterator pos;
				std::vector<node*> cache;
				for (iterator iter = b; iter != e; ++iter) {
					pos = find_(*iter, pos.node_, nullptr, &cache);
					if (pos == end()) {
						pos = insert_(*iter, cache);
					}
				}
			}

			int random_height_() {
				int height = 1;
				for (; height < max_height_ && (random_generator_() % branching_ == 0); height++);

				return height;
			}

		private:
			/*
			* 最大层高
			*/
			static constexpr int max_height_ = 32;
			/*
			* 分层概率
			*/
			static constexpr int branching_ = 4;
			/*
			* 当前层高
			*/
			int current_height_ = 1;
			/*
			* 元素个数
			*/
			std::size_t element_num_ = 0;
			/*
			* 尾指针
			*/
			node* tail_ = nullptr;
			/*
			* 头指针
			*/
			node* head_;
			/*
			* 随机生成器
			*/
			std::mt19937 random_generator_;
			//排序方式
			Compare compare_;
			/*
			* 当前排序,1.升序,0.初始化,-1降序
			*/
			int is_asc_ = 0;
			/*
			* key,score映射
			*/
			std::unordered_map<Key, Score> kvmap_;
		};
	}
}
#endif
