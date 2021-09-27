#ifndef SHYNET_UTILS_SKIPLIST_H
#define SHYNET_UTILS_SKIPLIST_H

#include <chrono>
#include <functional>
#include <iostream>
#include <random>
#include <vector>

namespace shynet {
	namespace utils {
		//
		// 跳表
		//
		template <typename Key, typename Score>
		class SkipList {

		public:
			using key_type = Key;
			using value_type = std::pair<const Key, Score>;
			using compare_fun = std::function<int(const value_type&, const value_type&)>;

		private:
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
					: height_(height), levels_(height) {}

				explicit node(int height, const value_type& kv)
					: kv_(kv), height_(height), levels_(height) {}

			private:
				value_type kv_;
				int height_;
				std::vector<level_item> levels_;
			};
		public:
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

			SkipList() : random_generator_(std::chrono::system_clock::now().time_since_epoch().count())
			{
				head_ = new node(max_height_);
			}

			SkipList(iterator first, iterator last) : random_generator_(std::chrono::system_clock::now().time_since_epoch().count())
			{
				head_ = new node(max_height_);
				insert_range_effective_(first, last);
			}

			SkipList(const SkipList& x)
				: head_(new node(max_height_))
			{
				insert_range_effective_(x.begin(), x.end());
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

			Score& operator[] (const value_type& k) {
				std::vector<node*> cache;
				iterator iter = find_(k, nullptr, nullptr, &cache);
				if (iter != end()) { return iter->second; }

				iter = insert_(k, cache);
				return iter->second;
			}

			Score& at(const value_type& k) {
				iterator iter = find_(k);
				if (iter == end()) {
					std::ostringstream err;
					err << "key=" << k.first << " score=" << k.second << " 不存在";
					THROW_EXCEPTION(err.str());
				}

				return iter->second;
			}

			const Score& at(const value_type& k) const {
				iterator iter = find_(k);
				if (iter == end()) {
					std::ostringstream err;
					err << "key=" << k.first << " score=" << k.second << " 不存在";
					THROW_EXCEPTION(err.str());
				}

				return iter->second;
			}

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

			std::pair<iterator, bool> insert(const value_type& val) {
				std::vector<node*> cache;
				iterator iter = find_(val, nullptr, nullptr, &cache);
				if (iter != end()) { return std::make_pair(iter, false); }

				iter = insert_(val, cache);
				return std::make_pair(iter, true);
			}

			iterator insert(iterator position, const value_type& val) {
				std::vector<node*> cache;
				iterator iter = find_(val, position.node_, nullptr, &cache);
				if (iter != end()) { return iter; }

				iter = find_(val, nullptr, position.node_, &cache);
				if (iter != end()) { return iter; }

				return insert_(val, cache);
			}

			void insert(iterator first, iterator last) {
				insert_range_effective_(first, last);
			}

			void swap(SkipList& x) {
				std::swap(head_, x.head_);
				std::swap(current_height_, x.current_height_);
				std::swap(element_num_, x.element_num_);
				std::swap(is_asc_, x.is_asc_);
			}

			iterator find(const value_type& k) const { return find_(k); }

			std::pair<iterator, iterator> find(const Score& min, const Score& max, size_t max_item_size = 100) {
				std::pair<iterator, iterator> rang = std::make_pair(end(), end());
				if (max >= min) {
					node* dummy = head_;
					for (int i = current_height_ - 1; i >= 0; i--) {
						while (dummy->levels_[i].next_ &&
							((is_asc_ == -1 && dummy->levels_[i].next_->kv_.second > max) ||
								(is_asc_ == 1 && dummy->levels_[i].next_->kv_.second < min))
							)
						{
							dummy = dummy->levels_[i].next_;
						}
					}
					size_t total = 0;
					dummy = dummy->levels_[0].next_;
					while (dummy &&
						((is_asc_ == -1 && dummy->kv_.second >= min) ||
							(is_asc_ == 1 && dummy->kv_.second <= max))
						)
					{
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

			iterator erase(iterator position) { return erase_(position); }

			iterator erase(const value_type& k) {
				iterator iter = find_(k);
				if (iter == end()) { return end(); }

				return erase_(iter);
			}

			void erase(iterator first, iterator last) {
				while (first != last) {
					first = erase(first);
				}
			}

			void debug_print() {
				std::cout << "<<<<<" << std::endl;
				for (int i = current_height_ - 1; i >= 0; i--) {
					std::cout << " * ";
					node* p = head_;
					for (; p->levels_[i].next_; p = p->levels_[i].next_) {
						std::cout << "(" << p->levels_[i].next_->kv_.first << ":" << p->levels_[i].next_->kv_.second
							<< " span:" << p->levels_[i].next_->levels_[i].span_ << ") ";
					}
					std::cout << std::endl;
				}
				std::cout << "-----" << std::endl;
			}
		private:
			iterator erase_(iterator it) {
				node* n = it.node_;
				iterator erase_next = end();
				for (int i = n->height_ - 1; i >= 0; i--) {
					erase_next = iterator(n->levels_[i].next_);
					n->levels_[i].prev_->levels_[i].next_ = n->levels_[i].next_;
					if (n->levels_[i].next_) {
						n->levels_[i].next_->levels_[i].prev_ = n->levels_[i].prev_;
					}
				}

				if (n->height_ != 1 && n->height_ == current_height_) {
					for (; (current_height_ > 1) && (head_->levels_[current_height_ - 1].next_ == nullptr); current_height_--);
				}

				if (n == tail_) { tail_ = n->levels_[0].prev_ == head_ ? nullptr : n->levels_[0].prev_; }

				element_num_--;
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

						int res = compare(k, dummy->levels_[i].next_->kv_);
						if (res == 0) { return iterator(dummy->levels_[i].next_); }
						else if (res < 0) { break; }
						else if (res > 0) { dummy = dummy->levels_[i].next_; }
					}
					if (cache) { (*cache)[i] = dummy; }
				}

				return end();
			}

			iterator insert_(const value_type& kv, std::vector<node*>& cache) {
				int height = random_height_();
				height = 1;
				/*if (kv.first == "h")
				{
					height = 5;
				}
				else {
					height = 1;
				}*/
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
				for (int i = height - 1; i >= 0; i--) {
					node* cn = cache[i];
					nn->levels_[i].span_ += cn->levels_[i].span_ + 1;
					nn->levels_[i].next_ = cn->levels_[i].next_;
					nn->levels_[i].prev_ = cn;
					if (cn->levels_[i].next_) { cn->levels_[i].next_->levels_[i].prev_ = nn; }

					cn->levels_[i].next_ = nn;
				}

				if (nn->levels_[0].next_ == nullptr) { tail_ = nn; }

				element_num_++;
				return iterator(nn);
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

			//排序方式
			static compare_fun compare;
		private:
			static constexpr int max_height_ = 12;
			static constexpr int branching_ = 4;
			int current_height_ = 1;
			std::size_t element_num_ = 0;
			node* tail_ = nullptr;
			node* head_;
			std::mt19937 random_generator_;
			/*
			* 当前排序,1.升序,0.初始化,-1降序
			*/
			int is_asc_ = 0;
		};

		template <typename Key, typename Score>
		static int default_compare(const std::pair<const Key, Score>& a, const std::pair<const Key, Score>& b) {
			if (a.first == b.first) {
				return 0;
			}
			else {
				//>升序 <降序
				if (a.second > b.second) {
					return 1;
				}
				else if (a.second == b.second) {
					if (a.first <= a.first)
					{
						return 1;
					}
					return -1;
				}
				else {
					return -1;
				}
			}
		}

		template <typename Key, typename Score>
		typename SkipList<Key, Score>::compare_fun SkipList<Key, Score>::compare = std::bind(default_compare<Key, Score>, std::placeholders::_1, std::placeholders::_2);
	}
}
#endif
